#ifndef DISASSEMBLY_HPP
#define DISASSEMBLY_HPP

#include "ftxui/component/component.hpp"
#include "emulator/olc6502.hpp"
#include "emulator/rambusdevice.hpp"

struct DisassemblyOption
{
    const olc6502 *model = nullptr;
    RamBusDevice  *ram = nullptr;
    ftxui::Ref<olc6502::addressType> start_address;
    ftxui::Ref<olc6502::addressType> end_address;
};

ftxui::Component disassembly(ftxui::Ref<DisassemblyOption> options);

#endif // DISASSEMBLY_HPP
