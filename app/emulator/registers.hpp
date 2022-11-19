#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include "flags.hpp"

struct Registers
{
    uint8_t  a = 0x00;
    uint8_t  x = 0x00;
    uint8_t  y = 0x00;
    uint8_t  stack_pointer   = 0x00;
    uint16_t program_counter = 0x00;
    uint8_t  status = 0x00;

    // Convenience functions to access status register
    uint8_t GetFlag(FLAGS6502 f) const
    {
        return ((status & f) > 0) ? 1 : 0;
    }
    void    SetFlag(FLAGS6502 f, bool v)
    {
        if (v)
            status |= f;
        else
            status &= ~f;
    }
};

#endif // REGISTERS_HPP
