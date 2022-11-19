#include "cliplaygroundapplication.h"
#include "ftxui/dom/node.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/component.hpp"
#include <QTimer>
#include <cstdlib>
#include <functional>

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

    clock_ticks = Renderer(
        [&]()
        {
            char buffer[16];

            snprintf(buffer, sizeof(buffer), "%.6u", computer.cpu()->clockTicks());
            return window( text("Clock Ticks"), text(buffer) ) | xflex;
        }
                         );
    reset_vector = Renderer(
        [&]()
        {
            char buffer[16];

            snprintf(buffer, sizeof(buffer), "$%.4X", computer.ram()->memory()[ olc6502::ResetJumpStartAddress ] |
                                                      (computer.ram()->memory()[ olc6502::ResetJumpStartAddress + 1] << 8) );
            return window( text("Reset Jump Address"), text(buffer) ) | xflex;
        });
    page_view_component = Renderer(
        [&]()
        {
            char buffer[16];

            snprintf(buffer, sizeof(buffer), "%.2X ", _memorypage_option.model->page());
            Element mpc_element = memory_page_component->Render();

            mpc_element->ComputeRequirement();
            return window( hbox({ text(" Memory Page: "), text(buffer) }), mpc_element ) | size(HEIGHT, EQUAL, mpc_element->requirement().min_y + 2);
        });

    disassembly_component = disassembly( &_disassembly_option );

    renderer = Renderer( Container::Vertical({ Container::Horizontal({ memory_page_component, disassembly_component, register_view_component}),
                                               Container::Horizontal({ step_button, next_instruction_button, run_button, pause_button, reset_button }) }),
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

    if ( !_simulation_control_thread.joinable() )
    {
        _simulation_running = true;
        _updated = false;

        std::thread start_simulation( &CLIPlaygroundApplication::simulationThread, this );

        _simulation_control_thread = std::move(start_simulation);
    }
}

void CLIPlaygroundApplication::simulationThread()
{
    assert(!_updated);

    while ( _simulation_running )
    {
        screen.Post( std::bind(&CLIPlaygroundApplication::updateTimeSlice, this) );

        {
            std::unique_lock lk{ _simulation_task_mutex };

            _cv.wait( lk, [this] { return _updated; });
        }
        {
            std::unique_lock lk{ _simulation_task_mutex };

            _updated = false;
        }
    }
}

void CLIPlaygroundApplication::updateTimeSlice()
{
    using namespace std::chrono_literals;

#if 0
    if ( !_simulation_running )
        return;
#endif

    auto start_time = std::chrono::steady_clock::now();
    auto frame_time = 16ms;

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

    screen.PostEvent(Event::Custom);

    {
        std::unique_lock lk{ _simulation_task_mutex };

        _updated = true;
    }
    _cv.notify_all();
}

void CLIPlaygroundApplication::onPauseButtonPressed()
{
    if ( !_simulation_running )
        return;

    if ( _simulation_control_thread.joinable() )
    {
        _simulation_running = false;
        {
            std::unique_lock lk{ _simulation_task_mutex };

            _updated = true;
        }
        _cv.notify_all();
        _simulation_control_thread.join();
    }
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

    screen.Loop( CatchEvent(renderer, std::bind(&CLIPlaygroundApplication::catchEvent, this, placeholders::_1) ));

    return EXIT_SUCCESS;
}

Element CLIPlaygroundApplication::generateView() const
{
    return window( text("6502 Playground") | hcenter, vbox({
                                                           hbox({ page_view_component->Render(),
                                                                  window( text("Disassembly"), disassembly_component->Render() ) | size(HEIGHT, EQUAL, 17),
                                                                  vbox({ register_view_component->Render(),
                                                                         clock_ticks->Render(),
                                                                         reset_vector->Render()}) }),
                                                           filler(),
                                                           separatorDouble(),
                                                         hbox({ step_button->Render(),
                                                                next_instruction_button->Render(),
                                                                run_button->Render(),
                                                                pause_button->Render(),
                                                               reset_button->Render() }) | size(HEIGHT, GREATER_THAN, 2)
                                                           })
                 );
}
