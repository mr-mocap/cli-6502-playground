#ifndef MEMORYPAGE_H
#define MEMORYPAGE_H

#include "ftxui/component/component.hpp"
#include "emulator/rambusdeviceview.hpp"
#include <memory>

using ftxui::Ref;
using ftxui::Component;
using ftxui::Event;
using ftxui::Decorator;
using std::shared_ptr;

struct MemoryPageOption
{
    shared_ptr<RamBusDeviceView> model;
    Ref<int>                     current_byte = -1;
    Ref<int>                     show_pc = -1;
    Ref<Event>                   previous_event;
};

Component MemoryPage(Ref<MemoryPageOption> option);

#endif // MEMORYPAGE_H
