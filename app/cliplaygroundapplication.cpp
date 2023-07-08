#include "cliplaygroundapplication.h"
#include "ftxui/dom/node.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/loop.hpp"
#include <QTimer>
#include <cstdlib>
#include <functional>
#include <QBuffer>
#include <QByteArray>
#include <chrono>

using namespace std;
using namespace ftxui;

inline uint8_t LowByteOf(uint16_t value)
{
    return static_cast<uint8_t>(value & 0X00FF);
}

inline uint8_t HighByteOf(uint16_t value)
{
    return static_cast<uint8_t>(value >> 8);
}

inline uint16_t MakeWord(uint8_t lo_byte, uint8_t hi_byte)
{
    return (hi_byte << 8) |  lo_byte;
}

CLIPlaygroundApplication *CLIPlaygroundApplication::_Instance = nullptr;

CLIPlaygroundApplication::CLIPlaygroundApplication(int &argc, char *argv[])
    :
    QCoreApplication(argc, argv),
    ram_view( std::make_shared<RamBusDeviceView>() ),
    register_view( std::make_shared<RegisterView>() ),
    _memorypage_option{ ram_view, -1, &_program_counter, &_previous_event },
    _disassembly_option{ computer.cpu(),
                         computer.ram(),
                         Ref<olc6502::addressType>(computer.cpu()->beginExecutingAtAddressAfterReset()),
                         Ref<olc6502::addressType>(computer.cpu()->beginExecutingAtAddressAfterReset() + static_cast<olc6502::addressType>(0x1000))}
{
    _ui_update_rates_dropdown_display_strings.reserve( _ui_update_rates.size() );
    for (const auto &[key, value] : _ui_update_rates )
        _ui_update_rates_dropdown_display_strings.push_back( key );

    setApplicationName("cli-6502-playground");
    setApplicationVersion("1.0.0");
    _Instance = this;
    computer.cpu()->connect(computer.cpu(), &olc6502::pcChanged,
                            [this](uint16_t new_value)
                            {
                                this->_program_counter = static_cast<int>(new_value);
                            });
    computer.ram()->connect(computer.ram(), &RamBusDevice::memoryChanged,
                            std::bind( &CLIPlaygroundApplication::memoryChanged, this, std::placeholders::_1, std::placeholders::_2 ));
    input_nmi_option.on_change = [this]()
    {
        computer.ram()->write( olc6502::NMIAddress    , LowByteOf( *input_nmi_option.data ) );
        computer.ram()->write( olc6502::NMIAddress + 1, HighByteOf( *input_nmi_option.data ) );
    };
    input_reset_option.on_change = [this]()
    {
        computer.ram()->write( olc6502::ResetJumpStartAddress    , LowByteOf( *input_reset_option.data ) );
        computer.ram()->write( olc6502::ResetJumpStartAddress + 1, HighByteOf( *input_reset_option.data ) );
    };
    input_irq_option.on_change = [this]()
    {
        computer.ram()->write( olc6502::IRQAddress    , LowByteOf( *input_irq_option.data ) );
        computer.ram()->write( olc6502::IRQAddress + 1, HighByteOf( *input_irq_option.data ) );
    };
}

CLIPlaygroundApplication::~CLIPlaygroundApplication()
{
    _Instance = nullptr;
}

void CLIPlaygroundApplication::Update()
{
    if (_Instance)
        _Instance->screen.PostEvent( Event::Custom );
}

void CLIPlaygroundApplication::setup_ui()
{
    *input_nmi_option.data = MakeWord( computer.ram()->read( olc6502::NMIAddress, false ),
                                        computer.ram()->read( olc6502::NMIAddress + 1, false ) );
    *input_reset_option.data = MakeWord( computer.ram()->read( olc6502::ResetJumpStartAddress, false ),
                                          computer.ram()->read( olc6502::ResetJumpStartAddress + 1, false ) );
    *input_irq_option.data = MakeWord( computer.ram()->read( olc6502::IRQAddress, false ),
                                        computer.ram()->read( olc6502::IRQAddress + 1, false ) );

    ram_view->setModel( computer.ram() );
    ram_view->setPage(0);

    register_view->setModel( computer.cpu() );

    memory_page_component = MemoryPage( &_memorypage_option );
    register_view_component = register_view->component();
    step_button = Button("Step", std::bind(&CLIPlaygroundApplication::onStepButtonPressed, this), ButtonOption::Border());
    next_instruction_button = Button("Next Instruction", std::bind(&CLIPlaygroundApplication::onNextInstructionButtonPressed, this), ButtonOption::Border());
    run_button = Button("Run", std::bind(&CLIPlaygroundApplication::onRunButtonPressed, this), ButtonOption::Border());
    pause_button = Button("Pause", std::bind(&CLIPlaygroundApplication::onPauseButtonPressed, this), ButtonOption::Border());
    reset_button = Button("Reset", std::bind(&CLIPlaygroundApplication::onResetButtonPressed, this), ButtonOption::Border());
    ui_update_rate_dropdown = Dropdown(&_ui_update_rates_dropdown_display_strings, &_selected_ui_rate);
    nmi_vector   = InputWord( &input_nmi_option );
    reset_vector = InputWord( &input_reset_option );
    irq_vector   = InputWord( &input_irq_option );

    clock_ticks = Renderer(
        [&]()
        {
            char buffer[16];

            snprintf(buffer, sizeof(buffer), "%06u", computer.cpu()->clockTicks());
            return window( text("Clock Ticks"), text(buffer) ) | xflex;
        } );
    system_vectors = Container::Vertical({ nmi_vector, reset_vector, irq_vector });

    disassembly_component = disassembly( &_disassembly_option );

    renderer = Renderer( Container::Vertical({ Container::Horizontal({ memory_page_component,
                                                                       disassembly_component,
                                                                       Container::Vertical( { register_view_component,
                                                                                              system_vectors } )
                                                                     }),
                                               Container::Horizontal({ step_button, next_instruction_button, run_button, pause_button, reset_button, ui_update_rate_dropdown }) }),
                         std::bind( &CLIPlaygroundApplication::generateView, this )
                       );
}

void CLIPlaygroundApplication::onStepButtonPressed()
{
    computer.stepClock();
    Update();
}

void CLIPlaygroundApplication::onNextInstructionButtonPressed()
{
    computer.stepInstruction();
    Update();
}

void CLIPlaygroundApplication::onRunButtonPressed()
{
    if ( _simulation_running )
        return;

    _simulation_running = true;
}

void CLIPlaygroundApplication::updateTimeSlice()
{
    using namespace std::chrono_literals;

    auto start_time = std::chrono::steady_clock::now();
    int hz = _ui_update_rates.at( _ui_update_rates_dropdown_display_strings[ _selected_ui_rate ] );
    std::chrono::milliseconds frame_time{ 1000 / hz };

    for (int count = 0; _simulation_running; ++count)
    {
        computer.stepClock();
        if ( (count % 50) == 0 )
        {
            auto time_diff = std::chrono::steady_clock::now() - start_time;

            if ( std::chrono::duration_cast<std::chrono::milliseconds>(time_diff) >= frame_time )
                break;
        }
    }
}

void CLIPlaygroundApplication::onPauseButtonPressed()
{
    if ( !_simulation_running )
        return;

    _simulation_running = false;
}

void CLIPlaygroundApplication::onResetButtonPressed()
{
    computer.cpu()->reset();
    computer.stepInstruction();
    screen.PostEvent(Event::Custom);
}

bool CLIPlaygroundApplication::catchEvent(Event event)
{
    _previous_event = event;

    if (event == Event::Escape)
    {
        screen.ExitLoopClosure()();
        return true;
    }

    return false;
}

int CLIPlaygroundApplication::mainLoop()
{
    step_button->TakeFocus();
#if 0
    computer.loadProgram( <Exact path to a .PRG file> );
#endif
    computer.stepInstruction();

    Component catch_app_events = CatchEvent(renderer,
                                            std::bind( &CLIPlaygroundApplication::catchEvent, this, placeholders::_1 ) );
    Loop loop( &screen, catch_app_events );

    while ( !loop.HasQuitted() )
    {
        loop.RunOnce();

        if ( _simulation_running )
        {
            // Run one "unit"/time-slice of the simulation
            updateTimeSlice();
            screen.PostEvent(Event::Custom);
        }
    }

    return EXIT_SUCCESS;
}

Element CLIPlaygroundApplication::generateView() const
{
    return window( text("6502 Playground") | hcenter, vbox({
                                                           hbox({ memory_page_component->Render(),
                                                                  window( text("Disassembly"), disassembly_component->Render() ) | size(HEIGHT, EQUAL, 17),
                                                                  vbox({ register_view_component->Render(),
                                                                         clock_ticks->Render(),
                                                                     window( text("System Vectors"),
                                                                            vbox({ hbox({ text("NMI:   "), nmi_vector->Render() }),
                                                                                   hbox({ text("RESET: "), reset_vector->Render() }),
                                                                                   hbox({ text("IRQ:   "), irq_vector->Render() })
                                                                                 }) )}) }),
                                                           filler(),
                                                           separatorDouble(),
                                                           hbox({ step_button->Render(),
                                                                  next_instruction_button->Render(),
                                                                  run_button->Render(),
                                                                  pause_button->Render(),
                                                                  reset_button->Render(),
                                                                  ui_update_rate_dropdown->Render() }) | size(HEIGHT, GREATER_THAN, 2)
                                                           })
                 );
}

void CLIPlaygroundApplication::memoryChanged(IBusDevice::addressType address, uint8_t data)
{
    if ( address == olc6502::ResetJumpStartAddress )
    {
        *input_reset_option.data = MakeWord( data, HighByteOf( *input_reset_option.data ) );
    }
    else if ( address == olc6502::ResetJumpStartAddress + 1 )
    {
        *input_reset_option.data = MakeWord( LowByteOf( *input_reset_option.data ), data );
    }
    else if ( address == olc6502::NMIAddress )
    {
        *input_nmi_option.data = MakeWord( data, HighByteOf( *input_nmi_option.data ) );
    }
    else if ( address == olc6502::NMIAddress + 1 )
    {
        *input_nmi_option.data = MakeWord( LowByteOf( *input_nmi_option.data ), data );
    }
    else if ( address == olc6502::IRQAddress )
    {
        *input_irq_option.data = MakeWord( data, HighByteOf( *input_irq_option.data ) );
    }
    else if ( address == olc6502::IRQAddress + 1 )
    {
        *input_irq_option.data = MakeWord( LowByteOf( *input_irq_option.data ), data );
    }
}
