#include "ibusdevice.hpp"


IBusDevice::IBusDevice(uint16_t  lower_address,
                       uint16_t  upper_address,
                       bool      writable,
                       bool      readable,
                       QObject  *parent)
    :
    QObject(parent),
    _lower_address_range(lower_address),
    _upper_address_range(upper_address),
    _writable(writable),
    _readable(readable)
{
}

IBusDevice::~IBusDevice()
{
}

bool IBusDevice::handlesAddress(uint16_t address) const
{
    return (address >= _lower_address_range) && (address <= _upper_address_range);
}

void IBusDevice::write(uint16_t address, uint8_t data)
{
    if (handlesAddress(address) && writable())
        writeImplementation(address, data);
}

uint8_t IBusDevice::read(uint16_t address, bool read_only)
{
    if (handlesAddress(address) && readable())
        return readImplementation(address, read_only);
    return 0x00;
}
