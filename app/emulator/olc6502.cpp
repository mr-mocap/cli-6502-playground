#include "olc6502.hpp"
#include <QDebug>
#include <ostream>

/*
    olc6502 - An emulation of the 6502/2A03 processor
    "Thanks Dad for believing computers were gonna be a big deal..." - javidx9

    License (OLC-3)
    ~~~~~~~~~~~~~~~

    Copyright 2018-2019 OneLoneCoder.com

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions or derivations of source code must retain the above
    copyright notice, this list of conditions and the following disclaimer.

    2. Redistributions or derivative works in binary form must reproduce
    the above copyright notice. This list of conditions and the following
    disclaimer must be reproduced in the documentation and/or other
    materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Background
    ~~~~~~~~~~
    I love this microprocessor. It was at the heart of two of my favourite
    machines, the BBC Micro, and the Nintendo Entertainment System, as well
    as countless others in that era. I learnt to program on the Model B, and
    I learnt to love games on the NES, so in many ways, this processor is
    why I am the way I am today.

    In February 2019, I decided to undertake a selfish personal project and
    build a NES emulator. Ive always wanted to, and as such I've avoided
    looking at source code for such things. This made making this a real
    personal challenge. I know its been done countless times, and very likely
    in far more clever and accurate ways than mine, but I'm proud of this.

    Datasheet: http://archive.6502.org/datasheets/rockwell_r650x_r651x.pdf

    Files: olc6502.h, olc6502.cpp

    Relevant Video: https://www.youtube.com/watch?v=8XmxKPJDGU0

    Links
    ~~~~~
    YouTube:	https://www.youtube.com/javidx9
                https://www.youtube.com/javidx9extra
    Discord:	https://discord.gg/WhwHUMV
    Twitter:	https://www.twitter.com/javidx9
    Twitch:		https://www.twitch.tv/javidx9
    GitHub:		https://www.github.com/onelonecoder
    Patreon:	https://www.patreon.com/javidx9
    Homepage:	https://www.onelonecodr.com

    Author
    ~~~~~~
    David Barr, aka javidx9, ©OneLoneCoder 2019
*/


olc6502::olc6502(QObject *parent)
    :
    QObject(parent),
    _executor{ _registers,
             [this](InstructionExecutor::addressType address, bool read_only)
             {
                 return read(address, read_only);
             },
             [this](InstructionExecutor::addressType address, uint8_t value)
             {
                 return write(address, value);
             },
             [this](InstructionExecutor::registerType new_value)
             {
                 emit aChanged(new_value);
             },
             [this](InstructionExecutor::registerType new_value)
             {
                 emit xChanged(new_value);
             },
             [this](InstructionExecutor::registerType new_value)
             {
                 emit yChanged(new_value);
             },
             [this](InstructionExecutor::addressType new_value)
             {
                 emit pcChanged(new_value);
             },
             [this](InstructionExecutor::registerType new_value)
             {
                 emit stackPointerChanged(new_value);
             },
             [this](InstructionExecutor::registerType new_value)
             {
                 emit statusChanged(new_value);
             }
           }
{
}

uint8_t olc6502::read(addressType address, bool read_only)
{
    return emit readSignal(address, read_only);
}

void olc6502::write(addressType address, uint8_t data)
{
    emit writeSignal(address, data);
}

// Forces the 6502 into a known state. This is hard-wired inside the CPU. The
// registers are set to 0x00, the status register is cleared except for unused
// bit which remains at 1. An absolute address is read from location 0xFFFC
// which contains a second address that the program counter is set to. This
// allows the programmer to jump to a known and programmable location in the
// memory to start executing from. Typically the programmer would set the value
// at location 0xFFFC at compile time.
void olc6502::reset()
{
    _executor.reset();
}

// Interrupt requests are a complex operation and only happen if the
// "disable interrupt" flag is 0. IRQs can happen at any time, but
// you dont want them to be destructive to the operation of the running
// program. Therefore the current instruction is allowed to finish
// (which I facilitate by doing the whole thing when cycles == 0) and
// then the current program counter is stored on the stack. Then the
// current status register is stored on the stack. When the routine
// that services the interrupt has finished, the status register
// and program counter can be restored to how they where before it
// occurred. This is impemented by the "RTI" instruction. Once the IRQ
// has happened, in a similar way to a reset, a programmable address
// is read form hard coded location 0xFFFE, which is subsequently
// set to the program counter.
void olc6502::irq()
{
    _executor.irq();
}


// A Non-Maskable Interrupt cannot be ignored. It behaves in exactly the
// same way as a regular IRQ, but reads the new program counter address
// form location 0xFFFA.
void olc6502::nmi()
{
    _executor.nmi();
}

// Perform one clock cycles worth of emulation
void olc6502::clock()
{
    _executor.clock();
}

bool olc6502::complete() const
{
    return _executor.complete();
}

void olc6502::setLog(bool value)
{
    if (value != _log)
    {
        _log = value;
        emit logChanged();
    }
}

auto olc6502::disassemble(addressType start, addressType stop) const -> disassemblyType
{
    return _executor.disassemble(start, stop);
}

olc6502::addressType olc6502::beginExecutingAtAddressAfterReset() const
{
    return static_cast<addressType>(read( ResetJumpStartAddress )) |
           static_cast<addressType>(read( ResetJumpStartAddress + 1 ) << 4);
}
