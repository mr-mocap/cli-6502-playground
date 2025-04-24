#ifndef MEMORYPAGE_H
#define MEMORYPAGE_H

#include "ftxui/component/component.hpp"
#include "ftxui/component/event.hpp"
#include "emulator/rambusdeviceview.hpp"
#include <memory>

struct MemoryPageOption
{
    std::shared_ptr<RamBusDeviceView> model;
    ftxui::Ref<int>                   current_byte = -1;
    ftxui::Ref<int>                   show_pc = -1;
    ftxui::Ref<ftxui::Event>          previous_event;
};

ftxui::Component MemoryPage(ftxui::Ref<MemoryPageOption> option);

#endif // MEMORYPAGE_H
