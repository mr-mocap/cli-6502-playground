#include "instructionexecutor.hpp"


InstructionExecutor::InstructionExecutor(Registers    &registers,
                                         readDelegate  read_signal,
                                         writeDelegate write_signal,
                                         registerValueChangedDelegate a_changed_signal,
                                         registerValueChangedDelegate x_changed_signal,
                                         registerValueChangedDelegate y_changed_signal,
                                         addressValueChangedDelegate  program_counter_changed_signal,
                                         registerValueChangedDelegate stack_pointer_changed_signal,
                                         registerValueChangedDelegate status_changed_signal
                                         )
    :
    _registers(registers),
    _read_delegate(read_signal),
    _write_delegate(write_signal),
    _a_changed(a_changed_signal),
    _x_changed(x_changed_signal),
    _y_changed(y_changed_signal),
    _stack_pointer_changed(stack_pointer_changed_signal),
    _program_counter_changed(program_counter_changed_signal),
    _status_changed(status_changed_signal)
{
    // Assembles the translation table. It's big, it's ugly, but it yields a convenient way
    // to emulate the 6502. I'm certain there are some "code-golf" strategies to reduce this
    // but I've deliberately kept it verbose for study and alteration

    // It is 16x16 entries. This gives 256 instructions. It is arranged to that the bottom
    // 4 bits of the instruction choose the column, and the top 4 bits choose the row.

    // For convenience to get function pointers to members of this class, I'm using this
    // or else it will be much much larger :D

    // The table is one big initializer list of initializer lists...
    using a = InstructionExecutor;
    _lookup =
    {
        { "BRK", &a::BRK, &a::IMM, 7 },{ "ORA", &a::ORA, &a::IZX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 3 },{ "ORA", &a::ORA, &a::ZP0, 3 },{ "ASL", &a::ASL, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PHP", &a::PHP, &a::IMP, 3 },{ "ORA", &a::ORA, &a::IMM, 2 },{ "ASL", &a::ASL, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::NOP, &a::IMP, 4 },{ "ORA", &a::ORA, &a::ABS, 4 },{ "ASL", &a::ASL, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
        { "BPL", &a::BPL, &a::REL, 2 },{ "ORA", &a::ORA, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "ORA", &a::ORA, &a::ZPX, 4 },{ "ASL", &a::ASL, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "CLC", &a::CLC, &a::IMP, 2 },{ "ORA", &a::ORA, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "ORA", &a::ORA, &a::ABX, 4 },{ "ASL", &a::ASL, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
        { "JSR", &a::JSR, &a::ABS, 6 },{ "AND", &a::AND, &a::IZX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "BIT", &a::BIT, &a::ZP0, 3 },{ "AND", &a::AND, &a::ZP0, 3 },{ "ROL", &a::ROL, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PLP", &a::PLP, &a::IMP, 4 },{ "AND", &a::AND, &a::IMM, 2 },{ "ROL", &a::ROL, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "BIT", &a::BIT, &a::ABS, 4 },{ "AND", &a::AND, &a::ABS, 4 },{ "ROL", &a::ROL, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
        { "BMI", &a::BMI, &a::REL, 2 },{ "AND", &a::AND, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "AND", &a::AND, &a::ZPX, 4 },{ "ROL", &a::ROL, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "SEC", &a::SEC, &a::IMP, 2 },{ "AND", &a::AND, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "AND", &a::AND, &a::ABX, 4 },{ "ROL", &a::ROL, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
        { "RTI", &a::RTI, &a::IMP, 6 },{ "EOR", &a::EOR, &a::IZX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 3 },{ "EOR", &a::EOR, &a::ZP0, 3 },{ "LSR", &a::LSR, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PHA", &a::PHA, &a::IMP, 3 },{ "EOR", &a::EOR, &a::IMM, 2 },{ "LSR", &a::LSR, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "JMP", &a::JMP, &a::ABS, 3 },{ "EOR", &a::EOR, &a::ABS, 4 },{ "LSR", &a::LSR, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
        { "BVC", &a::BVC, &a::REL, 2 },{ "EOR", &a::EOR, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "EOR", &a::EOR, &a::ZPX, 4 },{ "LSR", &a::LSR, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "CLI", &a::CLI, &a::IMP, 2 },{ "EOR", &a::EOR, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "EOR", &a::EOR, &a::ABX, 4 },{ "LSR", &a::LSR, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
        { "RTS", &a::RTS, &a::IMP, 6 },{ "ADC", &a::ADC, &a::IZX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 3 },{ "ADC", &a::ADC, &a::ZP0, 3 },{ "ROR", &a::ROR, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PLA", &a::PLA, &a::IMP, 4 },{ "ADC", &a::ADC, &a::IMM, 2 },{ "ROR", &a::ROR, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "JMP", &a::JMP, &a::IND, 5 },{ "ADC", &a::ADC, &a::ABS, 4 },{ "ROR", &a::ROR, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
        { "BVS", &a::BVS, &a::REL, 2 },{ "ADC", &a::ADC, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "ADC", &a::ADC, &a::ZPX, 4 },{ "ROR", &a::ROR, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "SEI", &a::SEI, &a::IMP, 2 },{ "ADC", &a::ADC, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "ADC", &a::ADC, &a::ABX, 4 },{ "ROR", &a::ROR, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
        { "???", &a::NOP, &a::IMP, 2 },{ "STA", &a::STA, &a::IZX, 6 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 6 },{ "STY", &a::STY, &a::ZP0, 3 },{ "STA", &a::STA, &a::ZP0, 3 },{ "STX", &a::STX, &a::ZP0, 3 },{ "???", &a::XXX, &a::IMP, 3 },{ "DEY", &a::DEY, &a::IMP, 2 },{ "???", &a::NOP, &a::IMP, 2 },{ "TXA", &a::TXA, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "STY", &a::STY, &a::ABS, 4 },{ "STA", &a::STA, &a::ABS, 4 },{ "STX", &a::STX, &a::ABS, 4 },{ "???", &a::XXX, &a::IMP, 4 },
        { "BCC", &a::BCC, &a::REL, 2 },{ "STA", &a::STA, &a::IZY, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 6 },{ "STY", &a::STY, &a::ZPX, 4 },{ "STA", &a::STA, &a::ZPX, 4 },{ "STX", &a::STX, &a::ZPY, 4 },{ "???", &a::XXX, &a::IMP, 4 },{ "TYA", &a::TYA, &a::IMP, 2 },{ "STA", &a::STA, &a::ABY, 5 },{ "TXS", &a::TXS, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 5 },{ "???", &a::NOP, &a::IMP, 5 },{ "STA", &a::STA, &a::ABX, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "???", &a::XXX, &a::IMP, 5 },
        { "LDY", &a::LDY, &a::IMM, 2 },{ "LDA", &a::LDA, &a::IZX, 6 },{ "LDX", &a::LDX, &a::IMM, 2 },{ "???", &a::XXX, &a::IMP, 6 },{ "LDY", &a::LDY, &a::ZP0, 3 },{ "LDA", &a::LDA, &a::ZP0, 3 },{ "LDX", &a::LDX, &a::ZP0, 3 },{ "???", &a::XXX, &a::IMP, 3 },{ "TAY", &a::TAY, &a::IMP, 2 },{ "LDA", &a::LDA, &a::IMM, 2 },{ "TAX", &a::TAX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "LDY", &a::LDY, &a::ABS, 4 },{ "LDA", &a::LDA, &a::ABS, 4 },{ "LDX", &a::LDX, &a::ABS, 4 },{ "???", &a::XXX, &a::IMP, 4 },
        { "BCS", &a::BCS, &a::REL, 2 },{ "LDA", &a::LDA, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 5 },{ "LDY", &a::LDY, &a::ZPX, 4 },{ "LDA", &a::LDA, &a::ZPX, 4 },{ "LDX", &a::LDX, &a::ZPY, 4 },{ "???", &a::XXX, &a::IMP, 4 },{ "CLV", &a::CLV, &a::IMP, 2 },{ "LDA", &a::LDA, &a::ABY, 4 },{ "TSX", &a::TSX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 4 },{ "LDY", &a::LDY, &a::ABX, 4 },{ "LDA", &a::LDA, &a::ABX, 4 },{ "LDX", &a::LDX, &a::ABY, 4 },{ "???", &a::XXX, &a::IMP, 4 },
        { "CPY", &a::CPY, &a::IMM, 2 },{ "CMP", &a::CMP, &a::IZX, 6 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "CPY", &a::CPY, &a::ZP0, 3 },{ "CMP", &a::CMP, &a::ZP0, 3 },{ "DEC", &a::DEC, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "INY", &a::INY, &a::IMP, 2 },{ "CMP", &a::CMP, &a::IMM, 2 },{ "DEX", &a::DEX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "CPY", &a::CPY, &a::ABS, 4 },{ "CMP", &a::CMP, &a::ABS, 4 },{ "DEC", &a::DEC, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
        { "BNE", &a::BNE, &a::REL, 2 },{ "CMP", &a::CMP, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "CMP", &a::CMP, &a::ZPX, 4 },{ "DEC", &a::DEC, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "CLD", &a::CLD, &a::IMP, 2 },{ "CMP", &a::CMP, &a::ABY, 4 },{ "NOP", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "CMP", &a::CMP, &a::ABX, 4 },{ "DEC", &a::DEC, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
        { "CPX", &a::CPX, &a::IMM, 2 },{ "SBC", &a::SBC, &a::IZX, 6 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "CPX", &a::CPX, &a::ZP0, 3 },{ "SBC", &a::SBC, &a::ZP0, 3 },{ "INC", &a::INC, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "INX", &a::INX, &a::IMP, 2 },{ "SBC", &a::SBC, &a::IMM, 2 },{ "NOP", &a::NOP, &a::IMP, 2 },{ "???", &a::SBC, &a::IMP, 2 },{ "CPX", &a::CPX, &a::ABS, 4 },{ "SBC", &a::SBC, &a::ABS, 4 },{ "INC", &a::INC, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
        { "BEQ", &a::BEQ, &a::REL, 2 },{ "SBC", &a::SBC, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "SBC", &a::SBC, &a::ZPX, 4 },{ "INC", &a::INC, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "SED", &a::SED, &a::IMP, 2 },{ "SBC", &a::SBC, &a::ABY, 4 },{ "NOP", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "SBC", &a::SBC, &a::ABX, 4 },{ "INC", &a::INC, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
    };
}

// The 6502 can address between 0x0000 - 0xFFFF. The high byte is often referred
// to as the "page", and the low byte is the offset into that page. This implies
// there are 256 pages, each containing 256 bytes.
//
// Several addressing modes have the potential to require an additional clock
// cycle if they cross a page boundary. This is combined with several instructions
// that enable this additional clock cycle. So each addressing function returns
// a flag saying it has potential, as does each instruction. If both instruction
// and address function return 1, then an additional clock cycle is required.


// Address Mode: Implied
// There is no additional data required for this instruction. The instruction
// does something very simple like like sets a status bit. However, we will
// target the accumulator, for instructions like PHA
uint8_t InstructionExecutor::IMP()
{
    _fetched = registers().a;
    return 0;
}

// Address Mode: Immediate
// The instruction expects the next byte to be used as a value, so we'll prep
// the read address to point to the next byte
uint8_t InstructionExecutor::IMM()
{
    _addr_abs = registers().program_counter++;
    return 0;
}

// Address Mode: Zero Page
// To save program bytes, zero page addressing allows you to absolutely address
// a location in first 0xFF bytes of address range. Clearly this only requires
// one byte instead of the usual two.
uint8_t InstructionExecutor::ZP0()
{
    _addr_abs = read(registers().program_counter);
    registers().program_counter++;
    _addr_abs &= 0x00FF;
    return 0;
}

// Address Mode: Zero Page with X Offset
// Fundamentally the same as Zero Page addressing, but the contents of the X Register
// is added to the supplied single byte address. This is useful for iterating through
// ranges within the first page.
uint8_t InstructionExecutor::ZPX()
{
    _addr_abs = (read(registers().program_counter) + registers().x);
    registers().program_counter++;
    _addr_abs &= 0x00FF;
    return 0;
}

// Address Mode: Zero Page with Y Offset
// Same as above but uses Y Register for offset
uint8_t InstructionExecutor::ZPY()
{
    _addr_abs = (read(registers().program_counter) + registers().y);
    registers().program_counter++;
    _addr_abs &= 0x00FF;
    return 0;
}

// Address Mode: Relative
// This address mode is exclusive to branch instructions. The address
// must reside within -128 to +127 of the branch instruction, i.e.
// you cant directly branch to any address in the addressable range.
uint8_t InstructionExecutor::REL()
{
    _addr_rel = read(registers().program_counter);
    registers().program_counter++;
    if (_addr_rel & 0x80)
        _addr_rel |= 0xFF00;
    return 0;
}

// Address Mode: Absolute
// A full 16-bit address is loaded and used
uint8_t InstructionExecutor::ABS()
{
    uint16_t lo = read(registers().program_counter);
    registers().program_counter++;
    uint16_t hi = read(registers().program_counter);
    registers().program_counter++;
    _addr_abs = (hi << 8) | lo;

    return 0;
}

// Address Mode: Absolute with X Offset
// Fundamentally the same as absolute addressing, but the contents of the X Register
// is added to the supplied two byte address. If the resulting address changes
// the page, an additional clock cycle is required
uint8_t InstructionExecutor::ABX()
{
    uint16_t lo = read(registers().program_counter);
    registers().program_counter++;
    uint16_t hi = read(registers().program_counter);
    registers().program_counter++;

    _addr_abs = (hi << 8) | lo;
    _addr_abs += registers().x;

    if ((_addr_abs & 0xFF00) != (hi << 8))
        return 1;
    else
        return 0;
}

// Address Mode: Absolute with Y Offset
// Fundamentally the same as absolute addressing, but the contents of the Y Register
// is added to the supplied two byte address. If the resulting address changes
// the page, an additional clock cycle is required
uint8_t InstructionExecutor::ABY()

{
    uint16_t lo = read(registers().program_counter);
    registers().program_counter++;
    uint16_t hi = read(registers().program_counter);
    registers().program_counter++;

    _addr_abs = (hi << 8) | lo;

    _addr_abs += registers().y;

    if ((_addr_abs & 0xFF00) != (hi << 8))
        return 1;
    else
        return 0;
}

// Note: The next 3 address modes use indirection (aka Pointers!)

// Address Mode: Indirect
// The supplied 16-bit address is read to get the actual 16-bit address. This is
// instruction is unusual in that it has a bug in the hardware! To emulate its
// function accurately, we also need to emulate this bug. If the low byte of the
// supplied address is 0xFF, then to read the high byte of the actual address
// we need to cross a page boundary. This doesnt actually work on the chip as
// designed, instead it wraps back around in the same page, yielding an
// invalid actual address
uint8_t InstructionExecutor::IND()
{
    uint16_t ptr_lo = read(registers().program_counter);
    registers().program_counter++;
    uint16_t ptr_hi = read(registers().program_counter);
    registers().program_counter++;

    uint16_t ptr = (ptr_hi << 8) | ptr_lo;

    if (ptr_lo == 0x00FF) // Simulate page boundary hardware bug
    {
        _addr_abs = (read(ptr & 0xFF00) << 8) | read(ptr + 0);
    }
    else // Behave normally
    {
        _addr_abs = (read(ptr + 1) << 8) | read(ptr + 0);
    }
    return 0;
}

// Address Mode: Indirect X
// The supplied 8-bit address is offset by X Register to index
// a location in page 0x00. The actual 16-bit address is read
// from this location
uint8_t InstructionExecutor::IZX()
{
    uint16_t t = read(registers().program_counter);
    registers().program_counter++;

    uint16_t lo = read((uint16_t)(t + (uint16_t)registers().x) & 0x00FF);
    uint16_t hi = read((uint16_t)(t + (uint16_t)registers().x + 1) & 0x00FF);

    _addr_abs = (hi << 8) | lo;

    return 0;
}

// Address Mode: Indirect Y
// The supplied 8-bit address indexes a location in page 0x00. From
// here the actual 16-bit address is read, and the contents of
// Y Register is added to it to offset it. If the offset causes a
// change in page then an additional clock cycle is required.
uint8_t InstructionExecutor::IZY()
{
    uint16_t t = read(registers().program_counter);
    registers().program_counter++;

    uint16_t lo = read(t & 0x00FF);
    uint16_t hi = read((t + 1) & 0x00FF);

    _addr_abs = (hi << 8) | lo;
    _addr_abs += registers().y;

    if ((_addr_abs & 0xFF00) != (hi << 8))
        return 1;
    else
        return 0;
}

// This function sources the data used by the instruction into
// a convenient numeric variable. Some instructions dont have to
// fetch data as the source is implied by the instruction. For example
// "INX" increments the X register. There is no additional data
// required. For all other addressing modes, the data resides at
// the location held within addr_abs, so it is read from there.
// Immediate adress mode exploits this slightly, as that has
// set addr_abs = pc + 1, so it fetches the data from the
// next byte for example "LDA $FF" just loads the accumulator with
// 256, i.e. no far reaching memory fetch is required. "fetched"
// is a variable global to the CPU, and is set by calling this
// function. It also returns it for convenience.
uint8_t InstructionExecutor::fetch()
{
    if (!(_lookup[_opcode].addrmode == &InstructionExecutor::IMP))
        _fetched = read(_addr_abs);
    return _fetched;
}

uint8_t InstructionExecutor::read(addressType address, bool read_only)
{
    return (_read_delegate) ? _read_delegate(address, read_only) : 0x00;
}

void InstructionExecutor::write(addressType address, uint8_t data)
{
    if (_write_delegate)
        _write_delegate(address, data);
}

// Forces the 6502 into a known state. This is hard-wired inside the CPU. The
// registers are set to 0x00, the status register is cleared except for unused
// bit which remains at 1. An absolute address is read from location 0xFFFC
// which contains a second address that the program counter is set to. This
// allows the programmer to jump to a known and programmable location in the
// memory to start executing from. Typically the programmer would set the value
// at location 0xFFFC at compile time.
void InstructionExecutor::reset()
{
    // Get address to set program counter to
    _addr_abs = ResetJumpStartAddress;
    uint16_t lo = read(_addr_abs + 0);
    uint16_t hi = read(_addr_abs + 1);

    // Set it
    registers().program_counter = (hi << 8) | lo;

    // Reset internal registers
    registers().a = 0;
    registers().x = 0;
    registers().y = 0;
    registers().stack_pointer = 0xFD;
    registers().status = 0x00 | U;

    // Clear internal helper variables
    _addr_rel = 0x0000;
    _addr_abs = 0x0000;
    _fetched = 0x00;

    // Reset takes time
    _cycles = 8;

    _a_changed(registers().a);
    _x_changed(registers().x);
    _y_changed(registers().y);
    _stack_pointer_changed(registers().stack_pointer);
    _status_changed(registers().status);
    _program_counter_changed(registers().program_counter);
}

void InstructionExecutor::irq()
{
    // If interrupts are allowed
    if (GetFlag(I) == 0)
    {
        // Push the program counter to the stack. It's 16-bits dont
        // forget so that takes two pushes
        write(0x0100 + registers().stack_pointer, (registers().program_counter >> 8) & 0x00FF);
        registers().stack_pointer--;
        write(0x0100 + registers().stack_pointer, registers().program_counter & 0x00FF);
        registers().stack_pointer--;

        // Then Push the status register to the stack
        SetFlag(B, 0);
        SetFlag(U, 1);
        SetFlag(I, 1);
        write(0x0100 + registers().stack_pointer, registers().status);
        registers().stack_pointer--;

        // Read new program counter location from fixed address
        _addr_abs = 0xFFFE;
        uint16_t lo = read(_addr_abs + 0);
        uint16_t hi = read(_addr_abs + 1);
        registers().program_counter = (hi << 8) | lo;

        // IRQs take time
        _cycles = 7;
    }
}

void InstructionExecutor::nmi()
{
    write(0x0100 + registers().stack_pointer, (registers().program_counter >> 8) & 0x00FF);
    registers().stack_pointer--;
    write(0x0100 + registers().stack_pointer, registers().program_counter & 0x00FF);
    registers().stack_pointer--;

    SetFlag(B, 0);
    SetFlag(U, 1);
    SetFlag(I, 1);
    write(0x0100 + registers().stack_pointer, registers().status);
    registers().stack_pointer--;

    _addr_abs = 0xFFFA;
    uint16_t lo = read(_addr_abs + 0);
    uint16_t hi = read(_addr_abs + 1);
    registers().program_counter = (hi << 8) | lo;

    _cycles = 8;
}

void InstructionExecutor::clock()
{
    // Each instruction requires a variable number of clock cycles to execute.
    // In my emulation, I only care about the final result and so I perform
    // the entire computation in one hit. In hardware, each clock cycle would
    // perform "microcode" style transformations of the CPUs state.
    //
    // To remain compliant with connected devices, it's important that the
    // emulation also takes "time" in order to execute instructions, so I
    // implement that delay by simply counting down the cycles required by
    // the instruction. When it reaches 0, the instruction is complete, and
    // the next one is ready to be executed.
    if (complete())
    {
        // Let's remember the previous values so we may only emit a single signal for whatever changed.
        auto registers_before = registers();

        // Read next instruction byte. This 8-bit value is used to index
        // the translation table to get the relevant information about
        // how to implement the instruction
        _opcode = read(registers().program_counter);

#if 0
        uint16_t log_pc = registers().program_counter; // For logging
#endif

        // Always set the unused status flag bit to 1
        SetFlag(U, true);

        // Increment program counter, we read the opcode byte
        registers().program_counter++;

        // Get Starting number of cycles
        _cycles = _lookup[_opcode].cycles;

        // Perform fetch of intermmediate data using the
        // required addressing mode
        uint8_t additional_cycle1 = (this->*_lookup[_opcode].addrmode)();

        // Perform operation
        uint8_t additional_cycle2 = (this->*_lookup[_opcode].operate)();

        // The addressmode and opcode may have altered the number
        // of cycles this instruction requires before its completed
        _cycles += (additional_cycle1 & additional_cycle2);

        if (additional_cycle2 > 1)
            _cycles += additional_cycle2 - 1; // Takes care of being in BCD mode

        // Always set the unused status flag bit to 1
        SetFlag(U, true);

#if 0
        if (log())
        {
            // This logger dumps every cycle the entire processor state for analysis.
            // This can be used for debugging the emulation, but has little utility
            // during emulation. Its also very slow, so only use if you have to.
            qDebug("%10d:%02d PC:%04X %s A:%02X X:%02X Y:%02X %s%s%s%s%s%s%s%s STKP:%02X\n",
                   clock_ticks, 0, log_pc, "XXX", registers().a, registers().x, registers().y,
                   GetFlag(N) ? "N" : ".",	GetFlag(V) ? "V" : ".",	GetFlag(U) ? "U" : ".",
                   GetFlag(B) ? "B" : ".",	GetFlag(D) ? "D" : ".",	GetFlag(I) ? "I" : ".",
                   GetFlag(Z) ? "Z" : ".",	GetFlag(C) ? "C" : ".",	registers().stack_pointer);
        }
#endif

        // Find out what has changed and emit the appropriate signals...
        if (registers().program_counter != registers_before.program_counter)
            _program_counter_changed(registers().program_counter);
        if (registers().status != registers_before.status)
            _status_changed(registers().status);
        if (registers().stack_pointer != registers_before.stack_pointer)
            _stack_pointer_changed(registers().stack_pointer);
        if (registers().a != registers_before.a)
            _a_changed(registers().a);
        if (registers().x != registers_before.x)
            _x_changed(registers().x);
        if (registers().y != registers_before.y)
            _y_changed(registers().y);
    }

    // Increment global clock count - This is actually unused unless logging is enabled
    // but I've kept it in because its a handy watch variable for debugging
    clock_ticks++;

    // Decrement the number of cycles remaining for this instruction
    _cycles--;
}

auto InstructionExecutor::disassemble(addressType start, addressType stop) const -> disassemblyType
{
    size_t  addr = start; // MUST be a value type that holds more values than start!
    uint8_t value = 0x00, lo = 0x00, hi = 0x00;
    size_t  line_addr = 0;
    disassemblyType mapLines;

    // A convenient utility to convert variables into
    // hex strings because "modern C++"'s method with
    // streams is atrocious
    auto hex = [](size_t n, uint8_t d)
    {
        std::string s(d, '0');

        for (; d > 0; --d, n >>= 4)
            s[d - 1] = "0123456789ABCDEF"[n & 0xF];
        return s;
    };

    // Starting at the specified address we read an instruction
    // byte, which in turn yields information from the lookup table
    // as to how many additional bytes we need to read and what the
    // addressing mode is. I need this info to assemble human readable
    // syntax, which is different depending upon the addressing mode

    // As the instruction is decoded, a std::string is assembled
    // with the readable output
    while (addr <= stop)
    {
        line_addr = addr;

        // Prefix line with instruction address
        std::string sInst = "$" + hex(addr, 4) + ": ";

        // Read instruction, and get its readable name
        uint8_t opcode = _read_delegate(addr, true); addr++;
        sInst += _lookup[opcode].name + " ";

        // Get oprands from desired locations, and form the
        // instruction based upon its addressing mode. These
        // routines mimmick the actual fetch routine of the
        // 6502 in order to get accurate data as part of the
        // instruction
        if (_lookup[opcode].addrmode == &InstructionExecutor::IMP)
        {
            //sInst += " {IMP}";
        }
        else if (_lookup[opcode].addrmode == &InstructionExecutor::IMM)
        {
            value = _read_delegate(addr, true); addr++;
            //sInst += "#$" + hex(value, 2) + " {IMM}";
            sInst += "#$" + hex(value, 2);
        }
        else if (_lookup[opcode].addrmode == &InstructionExecutor::ZP0)
        {
            lo = _read_delegate(addr, true); addr++;
            hi = 0x00;
            //sInst += "$" + hex(lo, 2) + " {ZP0}";
            sInst += "$" + hex(lo, 2);
        }
        else if (_lookup[opcode].addrmode == &InstructionExecutor::ZPX)
        {
            lo = _read_delegate(addr, true); addr++;
            hi = 0x00;
            //sInst += "$" + hex(lo, 2) + ", X {ZPX}";
            sInst += "$" + hex(lo, 2) + ", X";
        }
        else if (_lookup[opcode].addrmode == &InstructionExecutor::ZPY)
        {
            lo = _read_delegate(addr, true); addr++;
            hi = 0x00;
            //sInst += "$" + hex(lo, 2) + ", Y {ZPY}";
            sInst += "$" + hex(lo, 2) + ", Y";
        }
        else if (_lookup[opcode].addrmode == &InstructionExecutor::IZX)
        {
            lo = _read_delegate(addr, true); addr++;
            hi = 0x00;
            //sInst += "($" + hex(lo, 2) + ", X) {IZX}";
            sInst += "($" + hex(lo, 2) + ", X)";
        }
        else if (_lookup[opcode].addrmode == &InstructionExecutor::IZY)
        {
            lo = _read_delegate(addr, true); addr++;
            hi = 0x00;
            //sInst += "($" + hex(lo, 2) + "), Y {IZY}";
            sInst += "($" + hex(lo, 2) + "), Y";
        }
        else if (_lookup[opcode].addrmode == &InstructionExecutor::ABS)
        {
            lo = _read_delegate(addr, true); addr++;
            hi = _read_delegate(addr, true); addr++;
            //sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + " {ABS}";
            sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4);
        }
        else if (_lookup[opcode].addrmode == &InstructionExecutor::ABX)
        {
            lo = _read_delegate(addr, true); addr++;
            hi = _read_delegate(addr, true); addr++;
            //sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", X {ABX}";
            sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", X";
        }
        else if (_lookup[opcode].addrmode == &InstructionExecutor::ABY)
        {
            lo = _read_delegate(addr, true); addr++;
            hi = _read_delegate(addr, true); addr++;
            //sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", Y {ABY}";
            sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", Y";
        }
        else if (_lookup[opcode].addrmode == &InstructionExecutor::IND)
        {
            lo = _read_delegate(addr, true); addr++;
            hi = _read_delegate(addr, true); addr++;
            //sInst += "($" + hex((uint16_t)(hi << 8) | lo, 4) + ") {IND}";
            sInst += "($" + hex((uint16_t)(hi << 8) | lo, 4) + ")";
        }
        else if (_lookup[opcode].addrmode == &InstructionExecutor::REL)
        {
            value = _read_delegate(addr, true); addr++;
            //sInst += "$" + hex(value, 2) + " [$" + hex(addr + value, 4) + "] {REL}";
            sInst += "$" + hex(value, 2) + "   [$" + hex(addr + (int8_t)value, 4) + "]";
        }

        // Add the formed string to a std::map, using the instruction's
        // address as the key. This makes it convenient to look for later
        // as the instructions are variable in length, so a straight up
        // incremental index is not sufficient.
        mapLines[line_addr] = sInst;
    }

    return mapLines;
}
///////////////////////////////////////////////////////////////////////////////

// INSTRUCTION IMPLEMENTATIONS

// Note: Ive started with the two most complicated instructions to emulate, which
// ironically is addition and subtraction! Ive tried to include a detailed
// explanation as to why they are so complex, yet so fundamental. Im also NOT
// going to do this through the explanation of 1 and 2's complement.

// Instruction: Add with Carry In
// Function:    A = A + M + C
// Flags Out:   C, V, N, Z
//
// Explanation:
// The purpose of this function is to add a value to the accumulator and a carry bit. If
// the result is > 255 there is an overflow setting the carry bit. Ths allows you to
// chain together ADC instructions to add numbers larger than 8-bits. This in itself is
// simple, however the 6502 supports the concepts of Negativity/Positivity and Signed Overflow.
//
// 10000100 = 128 + 4 = 132 in normal circumstances, we know this as unsigned and it allows
// us to represent numbers between 0 and 255 (given 8 bits). The 6502 can also interpret
// this word as something else if we assume those 8 bits represent the range -128 to +127,
// i.e. it has become signed.
//
// Since 132 > 127, it effectively wraps around, through -128, to -124. This wraparound is
// called overflow, and this is a useful to know as it indicates that the calculation has
// gone outside the permissable range, and therefore no longer makes numeric sense.
//
// Note the implementation of ADD is the same in binary, this is just about how the numbers
// are represented, so the word 10000100 can be both -124 and 132 depending upon the
// context the programming is using it in. We can prove this!
//
//  10000100 =  132  or  -124
// +00010001 = + 17      + 17
//  ========    ===       ===     See, both are valid additions, but our interpretation of
//  10010101 =  149  or  -107     the context changes the value, not the hardware!
//
// In principle under the -128 to 127 range:
// 10000000 = -128, 11111111 = -1, 00000000 = 0, 00000000 = +1, 01111111 = +127
// therefore negative numbers have the most significant set, positive numbers do not
//
// To assist us, the 6502 can set the overflow flag, if the result of the addition has
// wrapped around. V <- ~(A^M) & A^(A+M+C) :D lol, let's work out why!
//
// Let's suppose we have A = 30, M = 10 and C = 0
//          A = 30 = 00011110
//          M = 10 = 00001010+
//     RESULT = 40 = 00101000
//
// Here we have not gone out of range. The resulting significant bit has not changed.
// So let's make a truth table to understand when overflow has occurred. Here I take
// the MSB of each component, where R is RESULT.
//
// A  M  R | V | A^R | A^M |~(A^M) |
// 0  0  0 | 0 |  0  |  0  |   1   |
// 0  0  1 | 1 |  1  |  0  |   1   |
// 0  1  0 | 0 |  0  |  1  |   0   |
// 0  1  1 | 0 |  1  |  1  |   0   |  so V = ~(A^M) & (A^R)
// 1  0  0 | 0 |  1  |  1  |   0   |
// 1  0  1 | 0 |  0  |  1  |   0   |
// 1  1  0 | 1 |  1  |  0  |   1   |
// 1  1  1 | 0 |  0  |  0  |   1   |
//
// We can see how the above equation calculates V, based on A, M and R. V was chosen
// based on the following hypothesis:
//       Positive Number + Positive Number = Negative Result -> Overflow
//       Negative Number + Negative Number = Positive Result -> Overflow
//       Positive Number + Negative Number = Either Result -> Cannot Overflow
//       Positive Number + Positive Number = Positive Result -> OK! No Overflow
//       Negative Number + Negative Number = Negative Result -> OK! NO Overflow

uint8_t InstructionExecutor::ADC()
{
    // Grab the data that we are adding to the accumulator
    fetch();

    // Check for BCD mode
    if (GetFlag(D))
    {
        BCDResult result = addBCD(registers().a, _fetched);

        _temp = result.sum;

        SetFlag(C, result.hi_nybble_carry);
        SetFlag(Z, (_temp & 0x00FF) == 0);

        // The signed Overflow flag is set based on all that up there! :D
        SetFlag(V, (~((uint16_t)registers().a ^ (uint16_t)_fetched) & ((uint16_t)registers().a ^ (uint16_t)_temp)) & 0x0080);

        // The negative flag is set to the most significant bit of the result.
        // it doesn't hurt to do the same thing for both BCD and binary mode.
        SetFlag(N, _temp & 0x80);

        // Load the result into the accumulator (it's 8-bit dont forget!)
        registers().a = _temp & 0x00FF;
    }
    else
    {
        // Add is performed in 16-bit domain for emulation to capture any
        // carry bit, which will exist in bit 8 of the 16-bit word
        _temp = (uint16_t)registers().a + (uint16_t)_fetched + (uint16_t)GetFlag(C);

        // The carry flag out exists in the high byte bit 0
        SetFlag(C, _temp > 255);

        // The Zero flag is set if the result is 0
        SetFlag(Z, (_temp & 0x00FF) == 0);

        // The signed Overflow flag is set based on all that up there! :D
        SetFlag(V, (~((uint16_t)registers().a ^ (uint16_t)_fetched) & ((uint16_t)registers().a ^ (uint16_t)_temp)) & 0x0080);

        // The negative flag is set to the most significant bit of the result
        SetFlag(N, _temp & 0x80);

        // Load the result into the accumulator (it's 8-bit dont forget!)
        registers().a = _temp & 0x00FF;
    }

    // This instruction has the potential to require an additional clock cycle
    return 1 + GetFlag(D);
}

// Instruction: Subtraction with Borrow In
// Function:    A = A - M - (1 - C)
// Flags Out:   C, V, N, Z
//
// Explanation:
// Given the explanation for ADC above, we can reorganise our data
// to use the same computation for addition, for subtraction by multiplying
// the data by -1, i.e. make it negative
//
// A = A - M - (1 - C)  ->  A = A + -1 * (M - (1 - C))  ->  A = A + (-M + 1 + C)
//
// To make a signed positive number negative, we can invert the bits and add 1
// (OK, I lied, a little bit of 1 and 2s complement :P)
//
//  5 = 00000101
// -5 = 11111010 + 00000001 = 11111011 (or 251 in our 0 to 255 range)
//
// The range is actually unimportant, because if I take the value 15, and add 251
// to it, given we wrap around at 256, the result is 10, so it has effectively
// subtracted 5, which was the original intention. (15 + 251) % 256 = 10
//
// Note that the equation above used (1-C), but this got converted to + 1 + C.
// This means we already have the +1, so all we need to do is invert the bits
// of M, the data(!) therfore we can simply add, exactly the same way we did
// before.

uint8_t InstructionExecutor::SBC()
{
    fetch();

    // Operating in 16-bit domain to capture carry out

    // Check for BCD mode
    if (GetFlag(D))
    {
        uint16_t value = complement(_fetched, GetFlag(D));
        BCDResult result = addBCD(registers().a, value + (uint16_t)GetFlag(C));

        _temp = result.sum;

        // Notice this is exactly the same as addition from here!
        SetFlag(C, result.hi_nybble_carry);
        SetFlag(Z, ((_temp & 0x00FF) == 0));
        SetFlag(V, (_temp ^ (uint16_t)registers().a) & (_temp ^ value) & 0x0080);
        SetFlag(N, _temp & 0x0080);
        registers().a = _temp & 0x00FF;
    }
    else
    {
        uint16_t value = complement(_fetched, GetFlag(D));

        // Notice this is exactly the same as addition from here!
        _temp = (uint16_t)registers().a + value + (uint16_t)GetFlag(C);
        SetFlag(C, _temp & 0xFF00);
        SetFlag(Z, ((_temp & 0x00FF) == 0));
        SetFlag(V, (_temp ^ (uint16_t)registers().a) & (_temp ^ value) & 0x0080);
        SetFlag(N, _temp & 0x0080);
        registers().a = _temp & 0x00FF;
    }

    return 1 + GetFlag(D);
}

// OK! Complicated operations are done! the following are much simpler
// and conventional. The typical order of events is:
// 1) Fetch the data you are working with
// 2) Perform calculation
// 3) Store the result in desired place
// 4) Set Flags of the status register
// 5) Return if instruction has potential to require additional
//    clock cycle


// Instruction: Bitwise Logic AND
// Function:    A = A & M
// Flags Out:   N, Z
uint8_t InstructionExecutor::AND()
{
    fetch();
    registers().a = registers().a & _fetched;
    SetFlag(Z, registers().a == 0x00);
    SetFlag(N, registers().a & 0x80);
    return 1;
}


// Instruction: Arithmetic Shift Left
// Function:    A = C <- (A << 1) <- 0
// Flags Out:   N, Z, C
uint8_t InstructionExecutor::ASL()
{
    fetch();
    _temp = (uint16_t)_fetched << 1;
    SetFlag(C, (_temp & 0xFF00) > 0);
    SetFlag(Z, (_temp & 0x00FF) == 0x00);
    SetFlag(N, _temp & 0x80);
    if (_lookup[_opcode].addrmode == &InstructionExecutor::IMP)
        registers().a = _temp & 0x00FF;
    else
        write(_addr_abs, _temp & 0x00FF);
    return 0;
}


// Instruction: Branch if Carry Clear
// Function:    if(C == 0) pc = address
uint8_t InstructionExecutor::BCC()
{
    if (GetFlag(C) == 0)
    {
        _cycles++;
        _addr_abs = registers().program_counter + _addr_rel;

        if((_addr_abs & 0xFF00) != (registers().program_counter & 0xFF00))
            _cycles++;

        registers().program_counter = _addr_abs;
    }
    return 0;
}


// Instruction: Branch if Carry Set
// Function:    if(C == 1) pc = address
uint8_t InstructionExecutor::BCS()
{
    if (GetFlag(C) == 1)
    {
        _cycles++;
        _addr_abs = registers().program_counter + _addr_rel;

        if ((_addr_abs & 0xFF00) != (registers().program_counter & 0xFF00))
            _cycles++;

        registers().program_counter = _addr_abs;
    }
    return 0;
}

// Instruction: Branch if Equal
// Function:    if(Z == 1) pc = address
uint8_t InstructionExecutor::BEQ()
{
    if (GetFlag(Z) == 1)
    {
        _cycles++;
        _addr_abs = registers().program_counter + _addr_rel;

        if ((_addr_abs & 0xFF00) != (registers().program_counter & 0xFF00))
            _cycles++;

        registers().program_counter = _addr_abs;
    }
    return 0;
}

uint8_t InstructionExecutor::BIT()
{
    fetch();
    _temp = registers().a & _fetched;
    SetFlag(Z, (_temp & 0x00FF) == 0x00);
    SetFlag(N, _fetched & (1 << 7));
    SetFlag(V, _fetched & (1 << 6));
    return 0;
}

// Instruction: Branch if Negative
// Function:    if(N == 1) pc = address
uint8_t InstructionExecutor::BMI()
{
    if (GetFlag(N) == 1)
    {
        _cycles++;
        _addr_abs = registers().program_counter + _addr_rel;

        if ((_addr_abs & 0xFF00) != (registers().program_counter & 0xFF00))
            _cycles++;

        registers().program_counter = _addr_abs;
    }
    return 0;
}


// Instruction: Branch if Not Equal
// Function:    if(Z == 0) pc = address
uint8_t InstructionExecutor::BNE()
{
    if (GetFlag(Z) == 0)
    {
        _cycles++;
        _addr_abs = registers().program_counter + _addr_rel;

        if ((_addr_abs & 0xFF00) != (registers().program_counter & 0xFF00))
            _cycles++;

        registers().program_counter = _addr_abs;
    }
    return 0;
}

// Instruction: Branch if Positive
// Function:    if(N == 0) pc = address
uint8_t InstructionExecutor::BPL()
{
    if (GetFlag(N) == 0)
    {
        _cycles++;
        _addr_abs = registers().program_counter + _addr_rel;

        if ((_addr_abs & 0xFF00) != (registers().program_counter & 0xFF00))
            _cycles++;

        registers().program_counter = _addr_abs;
    }
    return 0;
}

// Instruction: Break
// Function:    Program Sourced Interrupt
uint8_t InstructionExecutor::BRK()
{
    registers().program_counter++;

    SetFlag(I, 1);
    write(0x0100 + registers().stack_pointer, (registers().program_counter >> 8) & 0x00FF);
    registers().stack_pointer--;
    write(0x0100 + registers().stack_pointer, registers().program_counter & 0x00FF);
    registers().stack_pointer--;

    SetFlag(B, 1);
    write(0x0100 + registers().stack_pointer, registers().status);
    registers().stack_pointer--;
    SetFlag(B, 0);

    registers().program_counter = (uint16_t)read(0xFFFE) | ((uint16_t)read(0xFFFF) << 8);
    return 0;
}

// Instruction: Branch if Overflow Clear
// Function:    if(V == 0) pc = address
uint8_t InstructionExecutor::BVC()
{
    if (GetFlag(V) == 0)
    {
        _cycles++;
        _addr_abs = registers().program_counter + _addr_rel;

        if ((_addr_abs & 0xFF00) != (registers().program_counter & 0xFF00))
            _cycles++;

        registers().program_counter = _addr_abs;
    }
    return 0;
}

// Instruction: Branch if Overflow Set
// Function:    if(V == 1) pc = address
uint8_t InstructionExecutor::BVS()
{
    if (GetFlag(V) == 1)
    {
        _cycles++;
        _addr_abs = registers().program_counter + _addr_rel;

        if ((_addr_abs & 0xFF00) != (registers().program_counter & 0xFF00))
            _cycles++;

        registers().program_counter = _addr_abs;
    }
    return 0;
}

// Instruction: Clear Carry Flag
// Function:    C = 0
uint8_t InstructionExecutor::CLC()
{
    SetFlag(C, false);
    return 0;
}

// Instruction: Clear Decimal Flag
// Function:    D = 0
uint8_t InstructionExecutor::CLD()
{
    SetFlag(D, false);
    return 0;
}

// Instruction: Disable Interrupts / Clear Interrupt Flag
// Function:    I = 0
uint8_t InstructionExecutor::CLI()
{
    SetFlag(I, false);
    return 0;
}


// Instruction: Clear Overflow Flag
// Function:    V = 0
uint8_t InstructionExecutor::CLV()
{
    SetFlag(V, false);
    return 0;
}

// Instruction: Compare Accumulator
// Function:    C <- A >= M      Z <- (A - M) == 0
// Flags Out:   N, C, Z
uint8_t InstructionExecutor::CMP()
{
    fetch();
    _temp = (uint16_t)registers().a - (uint16_t)_fetched;
    SetFlag(C, registers().a >= _fetched);
    SetFlag(Z, (_temp & 0x00FF) == 0x0000);
    SetFlag(N, _temp & 0x0080);
    return 1;
}


// Instruction: Compare X Register
// Function:    C <- X >= M      Z <- (X - M) == 0
// Flags Out:   N, C, Z
uint8_t InstructionExecutor::CPX()
{
    fetch();
    _temp = (uint16_t)registers().x - (uint16_t)_fetched;
    SetFlag(C, registers().x >= _fetched);
    SetFlag(Z, (_temp & 0x00FF) == 0x0000);
    SetFlag(N, _temp & 0x0080);
    return 0;
}

// Instruction: Compare Y Register
// Function:    C <- Y >= M      Z <- (Y - M) == 0
// Flags Out:   N, C, Z
uint8_t InstructionExecutor::CPY()
{
    fetch();
    _temp = (uint16_t)registers().y - (uint16_t)_fetched;
    SetFlag(C, registers().y >= _fetched);
    SetFlag(Z, (_temp & 0x00FF) == 0x0000);
    SetFlag(N, _temp & 0x0080);
    return 0;
}

// Instruction: Decrement Value at Memory Location
// Function:    M = M - 1
// Flags Out:   N, Z
uint8_t InstructionExecutor::DEC()
{
    fetch();
    _temp = _fetched - 1;
    write(_addr_abs, _temp & 0x00FF);
    SetFlag(Z, (_temp & 0x00FF) == 0x0000);
    SetFlag(N, _temp & 0x0080);
    return 0;
}

// Instruction: Decrement X Register
// Function:    X = X - 1
// Flags Out:   N, Z
uint8_t InstructionExecutor::DEX()
{
    registers().x--;
    SetFlag(Z, registers().x == 0x00);
    SetFlag(N, registers().x & 0x80);
    return 0;
}


// Instruction: Decrement Y Register
// Function:    Y = Y - 1
// Flags Out:   N, Z
uint8_t InstructionExecutor::DEY()
{
    registers().y--;
    SetFlag(Z, registers().y == 0x00);
    SetFlag(N, registers().y & 0x80);
    return 0;
}


// Instruction: Bitwise Logic XOR
// Function:    A = A xor M
// Flags Out:   N, Z
uint8_t InstructionExecutor::EOR()
{
    fetch();
    registers().a = registers().a ^ _fetched;
    SetFlag(Z, registers().a == 0x00);
    SetFlag(N, registers().a & 0x80);
    return 1;
}

// Instruction: Increment Value at Memory Location
// Function:    M = M + 1
// Flags Out:   N, Z
uint8_t InstructionExecutor::INC()
{
    fetch();
    _temp = _fetched + 1;
    write(_addr_abs, _temp & 0x00FF);
    SetFlag(Z, (_temp & 0x00FF) == 0x0000);
    SetFlag(N, _temp & 0x0080);
    return 0;
}


// Instruction: Increment X Register
// Function:    X = X + 1
// Flags Out:   N, Z
uint8_t InstructionExecutor::INX()
{
    registers().x++;
    SetFlag(Z, registers().x == 0x00);
    SetFlag(N, registers().x & 0x80);
    return 0;
}


// Instruction: Increment Y Register
// Function:    Y = Y + 1
// Flags Out:   N, Z
uint8_t InstructionExecutor::INY()
{
    registers().y++;
    SetFlag(Z, registers().y == 0x00);
    SetFlag(N, registers().y & 0x80);
    return 0;
}


// Instruction: Jump To Location
// Function:    pc = address
uint8_t InstructionExecutor::JMP()
{
    registers().program_counter = _addr_abs;
    return 0;
}


// Instruction: Jump To Sub-Routine
// Function:    Push current pc to stack, pc = address
uint8_t InstructionExecutor::JSR()
{
    registers().program_counter--;

    write(0x0100 + registers().stack_pointer, (registers().program_counter >> 8) & 0x00FF);
    registers().stack_pointer--;
    write(0x0100 + registers().stack_pointer, registers().program_counter & 0x00FF);
    registers().stack_pointer--;

    registers().program_counter = _addr_abs;
    return 0;
}

// Instruction: Load The Accumulator
// Function:    A = M
// Flags Out:   N, Z
uint8_t InstructionExecutor::LDA()
{
    fetch();
    registers().a = _fetched;
    SetFlag(Z, registers().a == 0x00);
    SetFlag(N, registers().a & 0x80);
    return 1;
}


// Instruction: Load The X Register
// Function:    X = M
// Flags Out:   N, Z
uint8_t InstructionExecutor::LDX()
{
    fetch();
    registers().x = _fetched;
    SetFlag(Z, registers().x == 0x00);
    SetFlag(N, registers().x & 0x80);
    return 1;
}


// Instruction: Load The Y Register
// Function:    Y = M
// Flags Out:   N, Z
uint8_t InstructionExecutor::LDY()
{
    fetch();
    registers().y = _fetched;
    SetFlag(Z, registers().y == 0x00);
    SetFlag(N, registers().y & 0x80);
    return 1;
}

uint8_t InstructionExecutor::LSR()
{
    fetch();
    SetFlag(C, _fetched & 0x0001);
    _temp = _fetched >> 1;
    SetFlag(Z, (_temp & 0x00FF) == 0x0000);
    SetFlag(N, _temp & 0x0080);
    if (_lookup[_opcode].addrmode == &InstructionExecutor::IMP)
        registers().a = _temp & 0x00FF;
    else
        write(_addr_abs, _temp & 0x00FF);
    return 0;
}

uint8_t InstructionExecutor::NOP()
{
    // Sadly not all NOPs are equal, Ive added a few here
    // based on https://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes
    // and will add more based on game compatibility, and ultimately
    // I'd like to cover all illegal opcodes too
    switch (_opcode) {
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC:
        return 1;
        break;
    }
    return 0;
}

// Instruction: Bitwise Logic OR
// Function:    A = A | M
// Flags Out:   N, Z
uint8_t InstructionExecutor::ORA()
{
    fetch();
    registers().a = registers().a | _fetched;
    SetFlag(Z, registers().a == 0x00);
    SetFlag(N, registers().a & 0x80);
    return 1;
}


// Instruction: Push Accumulator to Stack
// Function:    A -> stack
uint8_t InstructionExecutor::PHA()
{
    write(0x0100 + registers().stack_pointer, registers().a);
    registers().stack_pointer--;
    return 0;
}


// Instruction: Push Status Register to Stack
// Function:    status -> stack
// Note:        Break flag is set to 1 before push
uint8_t InstructionExecutor::PHP()
{
    write(0x0100 + registers().stack_pointer, registers().status | B | U);
    SetFlag(B, 0);
    SetFlag(U, 0);
    registers().stack_pointer--;
    return 0;
}

// Instruction: Pop Accumulator off Stack
// Function:    A <- stack
// Flags Out:   N, Z
uint8_t InstructionExecutor::PLA()
{
    registers().stack_pointer++;
    registers().a = read(0x0100 + registers().stack_pointer);
    SetFlag(Z, registers().a == 0x00);
    SetFlag(N, registers().a & 0x80);
    return 0;
}


// Instruction: Pop Status Register off Stack
// Function:    Status <- stack
uint8_t InstructionExecutor::PLP()
{
    registers().stack_pointer++;
    registers().status = read(0x0100 + registers().stack_pointer);
    SetFlag(U, 1);
    return 0;
}

uint8_t InstructionExecutor::ROL()
{
    fetch();
    _temp = (uint16_t)(_fetched << 1) | GetFlag(C);
    SetFlag(C, _temp & 0xFF00);
    SetFlag(Z, (_temp & 0x00FF) == 0x0000);
    SetFlag(N, _temp & 0x0080);
    if (_lookup[_opcode].addrmode == &InstructionExecutor::IMP)
        registers().a = _temp & 0x00FF;
    else
        write(_addr_abs, _temp & 0x00FF);
    return 0;
}

uint8_t InstructionExecutor::ROR()
{
    fetch();
    _temp = (uint16_t)(GetFlag(C) << 7) | (_fetched >> 1);
    SetFlag(C, _fetched & 0x01);
    SetFlag(Z, (_temp & 0x00FF) == 0x00);
    SetFlag(N, _temp & 0x0080);
    if (_lookup[_opcode].addrmode == &InstructionExecutor::IMP)
        registers().a = _temp & 0x00FF;
    else
        write(_addr_abs, _temp & 0x00FF);
    return 0;
}

uint8_t InstructionExecutor::RTI()
{
    registers().stack_pointer++;
    registers().status = read(0x0100 + registers().stack_pointer);
    registers().status &= ~B;
    registers().status &= ~U;

    registers().stack_pointer++;
    registers().program_counter = (uint16_t)read(0x0100 + registers().stack_pointer);
    registers().stack_pointer++;
    registers().program_counter |= (uint16_t)read(0x0100 + registers().stack_pointer) << 8;
    return 0;
}

uint8_t InstructionExecutor::RTS()
{
    registers().stack_pointer++;
    registers().program_counter = (uint16_t)read(0x0100 + registers().stack_pointer);
    registers().stack_pointer++;
    registers().program_counter |= (uint16_t)read(0x0100 + registers().stack_pointer) << 8;

    registers().program_counter++;
    return 0;
}

// Instruction: Set Carry Flag
// Function:    C = 1
uint8_t InstructionExecutor::SEC()
{
    SetFlag(C, true);
    return 0;
}


// Instruction: Set Decimal Flag
// Function:    D = 1
uint8_t InstructionExecutor::SED()
{
    SetFlag(D, true);
    return 0;
}


// Instruction: Set Interrupt Flag / Enable Interrupts
// Function:    I = 1
uint8_t InstructionExecutor::SEI()
{
    SetFlag(I, true);
    return 0;
}


// Instruction: Store Accumulator at Address
// Function:    M = A
uint8_t InstructionExecutor::STA()
{
    write(_addr_abs, registers().a);
    return 0;
}


// Instruction: Store X Register at Address
// Function:    M = X
uint8_t InstructionExecutor::STX()
{
    write(_addr_abs, registers().x);
    return 0;
}


// Instruction: Store Y Register at Address
// Function:    M = Y
uint8_t InstructionExecutor::STY()
{
    write(_addr_abs, registers().y);
    return 0;
}

// Instruction: Transfer Accumulator to X Register
// Function:    X = A
// Flags Out:   N, Z
uint8_t InstructionExecutor::TAX()
{
    registers().x = registers().a;
    SetFlag(Z, registers().x == 0x00);
    SetFlag(N, registers().x & 0x80);
    return 0;
}


// Instruction: Transfer Accumulator to Y Register
// Function:    Y = A
// Flags Out:   N, Z
uint8_t InstructionExecutor::TAY()
{
    registers().y = registers().a;
    SetFlag(Z, registers().y == 0x00);
    SetFlag(N, registers().y & 0x80);
    return 0;
}


// Instruction: Transfer Stack Pointer to X Register
// Function:    X = stack pointer
// Flags Out:   N, Z
uint8_t InstructionExecutor::TSX()
{
    registers().x = registers().stack_pointer;
    SetFlag(Z, registers().x == 0x00);
    SetFlag(N, registers().x & 0x80);
    return 0;
}


// Instruction: Transfer X Register to Accumulator
// Function:    A = X
// Flags Out:   N, Z
uint8_t InstructionExecutor::TXA()
{
    registers().a = registers().x;
    SetFlag(Z, registers().a == 0x00);
    SetFlag(N, registers().a & 0x80);
    return 0;
}


// Instruction: Transfer X Register to Stack Pointer
// Function:    stack pointer = X
uint8_t InstructionExecutor::TXS()
{
    registers().stack_pointer = registers().x;
    return 0;
}

// Instruction: Transfer Y Register to Accumulator
// Function:    A = Y
// Flags Out:   N, Z
uint8_t InstructionExecutor::TYA()
{
    registers().a = registers().y;
    SetFlag(Z, registers().a == 0x00);
    SetFlag(N, registers().a & 0x80);
    return 0;
}


// This function captures illegal opcodes
uint8_t InstructionExecutor::XXX()
{
    return 0;
}

InstructionExecutor::BCDResult InstructionExecutor::addBCD(uint8_t left, uint8_t right)
{
    // First add low nybbles to get the first digit...
    uint16_t low_nybble_result = ((uint16_t)left  & 0x000F) +
                                 ((uint16_t)right & 0x000F);
    uint16_t low_nybble_carry = (low_nybble_result > 0x09) ? 0x0001 : 0x0000;

    // Check for the wrap-around...
    if (low_nybble_carry)
        low_nybble_result -= 10;

    // Then add high nybbles to get the second digit...
    uint16_t hi_nybble_result = ((uint16_t)left  >> 4) +
                                ((uint16_t)right >> 4) +
                                low_nybble_carry;
    uint16_t hi_nybble_carry = (hi_nybble_result > 0x09) ? 0x0001 : 0x0000;

    if (hi_nybble_carry)
        hi_nybble_result -= 10;

    return { static_cast<uint8_t>(low_nybble_result | (hi_nybble_result << 4)), static_cast<bool>(hi_nybble_carry) }; // Splice the nybbles back together
}
