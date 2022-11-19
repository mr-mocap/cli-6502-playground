#ifndef DISASSEMBLYVIEW_HPP
#define DISASSEMBLYVIEW_HPP

#include "ftxui/component/component.hpp"
#include "emulator/olc6502.hpp"
#include "emulator/rambusdevice.hpp"

ftxui::Element disassemblyview(RamBusDevice *ram, const olc6502 *model);

#endif // DISASSEMBLYVIEW_HPP
