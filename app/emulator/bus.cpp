#include "bus.hpp"

Bus::Bus(QObject *parent)
    :
    QObject(parent)
{
}

void Bus::write(addressType address, uint8_t data)
{
    emit busWritten(address, data);
}

uint8_t Bus::read(addressType address, bool read_only)
{
    return emit busRead(address, read_only);
}
