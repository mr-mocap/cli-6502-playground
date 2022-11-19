#include "computer.hpp"
#include <QTimer>
#include <sstream>


Computer::Computer(QObject *parent) : QObject(parent)
{
    // Read signals
    QObject::connect(&_cpu, &olc6502::readSignal,
                     &_bus, &Bus::read);
    QObject::connect(&_bus,    &Bus::busRead,
                     &_memory, &RamBusDevice::read);

    // Write signals
    QObject::connect(&_cpu, &olc6502::writeSignal,
                     &_bus, &Bus::write);
    QObject::connect(&_bus,    &Bus::busWritten,
                     &_memory, &RamBusDevice::write);
    _clock.setInterval(16);
    _clock.setSingleShot(false);
    QObject::connect(&_clock, &QTimer::timeout,
                     this,    &Computer::timerTimeout);
}

void Computer::startClock()
{
    _clock.start();
}

void Computer::stopClock()
{
    _clock.stop();
}

void Computer::stepClock()
{
    _cpu.clock();
}

void Computer::stepInstruction(int number_of_instructions)
{
    for ( ; number_of_instructions > 0; --number_of_instructions )
    {
        stepClock();
        while ( !_cpu.complete() )
            stepClock();
    }
}

void Computer::timerTimeout()
{
    stepClock();
}

void Computer::loadProgram()
{
    // Load Program (assembled at https://www.masswerk.at/6502/assembler.html)
    /*
       *=$8000
       LDX #10
       STX $0000
       LDX #3
       STX $0001
       LDY $0000
       LDA #0
       CLC
       loop
       ADC $0001
       DEY
       BNE loop
       STA $0002
       NOP
       NOP
       NOP
   */

    // Convert hex string into bytes for RAM
    std::stringstream ss;

    ss << "A2 0A 8E 00 00 A2 03 8E 01 00 AC 00 00 A9 00 18 6D 01 00 88 D0 FA 8D 02 00 EA EA EA";

    for (uint16_t nOffset = 0x8000; !ss.eof(); ++nOffset)
    {
        std::string b;

        ss >> b;
        _memory.write(nOffset, (uint8_t)std::stoul(b, nullptr, 16));
    }

    // Set Reset Vector
    _memory.write(0xFFFC, 0x00);
    _memory.write(0xFFFD, 0x80);

    // Reset
    _cpu.reset();
}
