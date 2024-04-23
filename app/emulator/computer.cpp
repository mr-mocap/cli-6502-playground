#include "computer.hpp"
#include <QTimer>
#include <sstream>

#include "io/io.hpp"


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

void Computer::load(const MemoryBlock &mb)
{
    int start_address = mb.first;

    for (uint8_t iCurrentByte : mb.second)
        _memory.write(start_address++, iCurrentByte);
}

void Computer::loadProgram(QString path)
{
    OptionalProgram input = ReadFromFile( path );

    if ( input )
    {
        for (const auto &memory_block : input.value().data)
        {
            load( memory_block );

            const int load_address = memory_block.first;

            // Set Reset Vector
            _memory.write(0xFFFC, load_address & 0xFF);
            _memory.write(0xFFFD, load_address >> 8);
        }

        // Reset
        _cpu.reset();
    }
}
