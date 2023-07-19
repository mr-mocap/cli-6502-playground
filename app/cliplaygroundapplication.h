#ifndef CLIPLAYGROUNDAPPLICATION_H
#define CLIPLAYGROUNDAPPLICATION_H

#include <QCoreApplication>
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "emulator/computer.hpp"
#include "emulator/rambusdeviceview.hpp"
#include "emulator/registerview.hpp"
#include "emulator/memorypage.hpp"
#include "emulator/disassembly.hpp"
#include "ui/components/inputnumber.hpp"
#include "ui/components/directorybrowser.hpp"
#include <memory>
#include <map>
#include <vector>
#include <string>

class CLIPlaygroundApplication : public QCoreApplication
{
    Q_OBJECT
public:
    CLIPlaygroundApplication(int &argc, char *argv[]);
   ~CLIPlaygroundApplication();

    static void Update();

    void setup_ui();

    int  mainLoop();

    Computer         computer;
    std::shared_ptr<RamBusDeviceView> ram_view;
    std::shared_ptr<RegisterView>     register_view;
    InputWordOption              input_nmi_option;
    InputWordOption              input_reset_option;
    InputWordOption              input_irq_option;
    InputDirectoryOption         load_file_option;
    ftxui::Component             memory_page_component;
    ftxui::Component             register_view_component;
    ftxui::Component             disassembly_component;
    ftxui::Component             clock_ticks;
    ftxui::Component             system_vectors;
    ftxui::Component             nmi_vector;
    ftxui::Component             reset_vector;
    ftxui::Component             irq_vector;
    ftxui::Component             step_button;
    ftxui::Component             run_button;
    ftxui::Component             pause_button;
    ftxui::Component             next_instruction_button;
    ftxui::Component             reset_button;
    ftxui::Component             load_file_button;
    ftxui::Component             ui_update_rate_dropdown;
    ftxui::Component             main_tab;
    ftxui::Component             main_container;
    ftxui::Component             depth_0_renderer;
    ftxui::Component             depth_1_renderer;
    ftxui::Component             renderer;
    ftxui::ScreenInteractive     screen{ ftxui::ScreenInteractive::Fullscreen() };
    int                          main_tab_selection = 0;

signals:

protected:
    MemoryPageOption  _memorypage_option;
    DisassemblyOption _disassembly_option;
    ftxui::Event      _previous_event;
    int               _program_counter = 0;
    bool              _simulation_running = false;
    int               _selected_ui_rate = 3;
    std::map<std::string, int> _ui_update_rates{ { " 5 Hz",  5 },
                                                 { "10 Hz", 10 },
                                                 { "20 Hz", 20 },
                                                 { "30 Hz", 30 },
                                                 { "40 Hz", 40 },
                                                 { "60 Hz", 60 },
                                                 { "72 Hz", 72 } };
    std::vector<std::string> _ui_update_rates_dropdown_display_strings;

    static CLIPlaygroundApplication *_Instance;

    void onStepButtonPressed();
    void onNextInstructionButtonPressed();
    void onRunButtonPressed();
    void onPauseButtonPressed();
    void onResetButtonPressed();
    void onLoadFileButtonPressed();
    void onLoadFileFinished(int button);
    bool catchEvent(ftxui::Event event);
    ftxui::Element generateView() const;
    void updateTimeSlice();
    void memoryChanged(IBusDevice::addressType address, uint8_t data);

protected slots:
};

#endif // CLIPLAYGROUNDAPPLICATION_H
