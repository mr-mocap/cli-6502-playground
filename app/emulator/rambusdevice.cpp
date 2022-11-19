#include "rambusdevice.hpp"
#include <algorithm>


RamBusDevice::RamBusDevice()
    :
    IBusDevice(0x0000, 0xFFFF, true, true)
{
    std::fill( std::begin(_data), std::end(_data), 0);
}

RamBusDevice::~RamBusDevice()
{
}

void RamBusDevice::writeImplementation(uint16_t address, uint8_t data)
{
    _data[address] = data;
    emit memoryChanged(address, data);
}

uint8_t RamBusDevice::readImplementation(uint16_t address, bool read_only)
{
    Q_UNUSED(read_only);

    return _data[address];
}
