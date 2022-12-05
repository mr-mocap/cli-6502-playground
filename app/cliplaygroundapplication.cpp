#include "cliplaygroundapplication.h"
#include "ftxui/dom/node.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/loop.hpp"
#include "emulator/inputnumber.hpp"
#include <QTimer>
#include <cstdlib>
#include <functional>
#include <QBuffer>
#include <QByteArray>
#include <chrono>

using namespace std;
using namespace ftxui;


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

    clock_ticks = Renderer(
        [&]()
        {
            char buffer[16];

            snprintf(buffer, sizeof(buffer), "%06u", computer.cpu()->clockTicks());
            return window( text("Clock Ticks"), text(buffer) ) | xflex;
        } );
    nmi_vector = Renderer(
        [&]()
        {
            char buffer[16];

            snprintf(buffer, sizeof(buffer), "$%04X", computer.ram()->memory()[ olc6502::NMIAddress ] |
                                                      (computer.ram()->memory()[ olc6502::NMIAddress + 1] << 8) );
            return hbox( text("NMI: "), filler(), text(buffer) ) | xflex;
        });
    irq_vector = Renderer(
        [&]()
        {
            char buffer[16];

            snprintf(buffer, sizeof(buffer), "$%04X", computer.ram()->memory()[ olc6502::IRQAddress ] |
                                                      (computer.ram()->memory()[ olc6502::IRQAddress + 1] << 8) );
            return hbox({ text("IRQ: "), filler(), text(buffer) }) | xflex;
        });
    reset_vector = Renderer(
        [&]()
        {
            char buffer[16];

            snprintf(buffer, sizeof(buffer), "$%04X", computer.ram()->memory()[ olc6502::ResetJumpStartAddress ] |
                                                      (computer.ram()->memory()[ olc6502::ResetJumpStartAddress + 1] << 8) );
            return hbox({ text("RESET: "), filler(), text(buffer) }) | xflex;
        });
    system_vectors = Container::Vertical({ nmi_vector, reset_vector, irq_vector });

    page_view_component = Renderer(
        [&]()
        {
            char buffer[16];

            snprintf(buffer, sizeof(buffer), "%02X ", _memorypage_option.model->page());
            Element mpc_element = memory_page_component->Render();

            mpc_element->ComputeRequirement();
            return window( hbox({ text(" Memory Page: "), text(buffer) }), mpc_element ) | size(HEIGHT, EQUAL, mpc_element->requirement().min_y + 2);
        });

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
    computer.loadProgram();
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
                                                           hbox({ page_view_component->Render(),
                                                                  window( text("Disassembly"), disassembly_component->Render() ) | size(HEIGHT, EQUAL, 17),
                                                                  vbox({ register_view_component->Render(),
                                                                         clock_ticks->Render(),
                                                                         window( text("System Vectors"), system_vectors->Render() )}) }),
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
