#ifndef CPU_HPP
#define CPU_HPP

#include <functional>


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

    Relevant Video: https://youtu.be/8XmxKPJDGU0

    Links
    ~~~~~
    YouTube:	https://www.youtube.com/javidx9
                https://www.youtube.com/javidx9extra
    Discord:	https://discord.gg/WhwHUMV
    Twitter:	https://www.twitter.com/javidx9
    Twitch:		https://www.twitch.tv/javidx9
    GitHub:		https://www.github.com/onelonecoder
    Patreon:	https://www.patreon.com/javidx9
    Homepage:	https://www.onelonecoder.com

    Author
    ~~~~~~
    David Barr, aka javidx9, ©OneLoneCoder 2019
*/

#include <QObject>
#include <QPointer>
#include <string>
#include <map>
#include "registers.hpp"
#include "instructionexecutor.hpp"


class olc6502 : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int a            READ property_a      NOTIFY aChanged)
    Q_PROPERTY(int x            READ property_x      NOTIFY xChanged)
    Q_PROPERTY(int y            READ property_y      NOTIFY yChanged)
    Q_PROPERTY(int stackPointer READ property_stkp   NOTIFY stackPointerChanged)
    Q_PROPERTY(int pc           READ property_pc     NOTIFY pcChanged)
    Q_PROPERTY(int status       READ property_status NOTIFY statusChanged)

    Q_PROPERTY(bool log         READ log             WRITE setLog NOTIFY logChanged)
public:
    using addressType = uint16_t;
    using disassemblyType = std::map<addressType, std::string>;

    Q_ENUM(FLAGS6502)

    explicit olc6502(QObject *parent = nullptr);

    void reset();

    void irq();
    void nmi();

    // Indicates the current instruction has completed by returning true. This is
    // a utility function to enable "step-by-step" execution, without manually
    // clocking every cycle
    bool complete() const;

    uint8_t a() const { return _registers.a; }
    uint8_t x() const { return _registers.x; }
    uint8_t y() const { return _registers.y; }

    uint16_t pc() const { return _registers.program_counter; }
    uint8_t  stackPointer() const { return _registers.stack_pointer; }
    uint8_t  status() const { return _registers.status; }

    const Registers &registers() const { return _registers; }
          Registers &registers()       { return _registers; }

    uint32_t clockTicks() const { return _executor.clock_ticks; }

    bool log() const { return _log; }
    void setLog(bool value);

    auto disassemble(addressType start, addressType stop) const -> disassemblyType;

    addressType beginExecutingAtAddressAfterReset() const;

    // The processor loads the 16-bit address contained at this address and then
    // begins executing instructions.
    static constexpr addressType ResetJumpStartAddress = InstructionExecutor::ResetJumpStartAddress;
    static constexpr addressType NMIAddress = InstructionExecutor::NMIAddress;
    static constexpr addressType IRQAddress = InstructionExecutor::IRQAddress;
public slots:
    void clock(); ///< Executes one clock tick

    uint8_t read(addressType address, bool read_only = false);
    uint8_t read(addressType address, bool read_only = false) const
    {
        return const_cast<olc6502 *>(this)->read(address, read_only);
    }
    void    write(addressType address, uint8_t data);

signals:
    uint8_t readSignal(addressType address, bool read_only);
    void    writeSignal(addressType address, uint8_t data);

    void aChanged(uint8_t new_value);
    void xChanged(uint8_t new_value);
    void yChanged(uint8_t new_value);
    void stackPointerChanged(uint8_t new_value);
    void pcChanged(uint16_t new_value);
    void statusChanged(uint8_t new_value);

    void logChanged();

private:
    // Assisstive variables to facilitate emulation
    Registers _registers;
    InstructionExecutor _executor;
    bool     _log = false;

    // These only exist to get around the QML type system.  It only really knows about
    // int, which is OK because in this case, all unsigned 8-bit values exist within the
    // positive half of an int.
    int property_a() { return static_cast<int>(a()); }
    int property_x() { return static_cast<int>(x()); }
    int property_y() { return static_cast<int>(y()); }
    int property_stkp() { return static_cast<int>(stackPointer()); }
    int property_pc() { return static_cast<int>(pc()); }
    int property_status() { return static_cast<int>(status()); }
};

#endif // CPU_HPP
