#ifndef INSTRUCTIONEXECUTOR_HPP
#define INSTRUCTIONEXECUTOR_HPP

#include <functional>
#include <map>
#include <vector>
#include <string>
#include "registers.hpp"


class InstructionExecutor
{
public:
    using addressType = uint16_t;
    using registerType = uint8_t;
    using readDelegate  = std::function<uint8_t (addressType, bool)>;
    using writeDelegate = std::function<void (addressType, uint8_t)>;
    using registerValueChangedDelegate = std::function<void (registerType)>;
    using addressValueChangedDelegate  = std::function<void (addressType)>;
    using disassemblyType = std::map<addressType, std::string>;

    // This structure and the following vector are used to compile and store
    // the opcode translation table. The 6502 can effectively have 256
    // different instructions. Each of these are stored in a table in numerical
    // order so they can be looked up easily, with no decoding required.
    // Each table entry holds:

    //	Pneumonic : A textual representation of the instruction (used for disassembly)
    //	Opcode Function: A function pointer to the implementation of the opcode
    //	Opcode Address Mode : A function pointer to the implementation of the
    //						  addressing mechanism used by the instruction
    //	Cycle Count : An integer that represents the base number of clock cycles the
    //				  CPU requires to perform the instruction
    struct INSTRUCTION
    {
        std::string name;
        uint8_t (InstructionExecutor::*operate)(void)  = nullptr;
        uint8_t (InstructionExecutor::*addrmode)(void) = nullptr;
        uint8_t cycles = 0;
    };

    InstructionExecutor() = delete;
    InstructionExecutor(Registers    &registers,
                        readDelegate  read_signal,
                        writeDelegate write_signal,
                        registerValueChangedDelegate a_changed_signal,
                        registerValueChangedDelegate x_changed_signal,
                        registerValueChangedDelegate y_changed_signal,
                        addressValueChangedDelegate  program_counter_changed_signal,
                        registerValueChangedDelegate stack_pointer_changed_signal,
                        registerValueChangedDelegate status_changed_signal);
    InstructionExecutor(const InstructionExecutor &) = delete;
    InstructionExecutor(InstructionExecutor &&) = delete;

    // Addressing Modes =============================================
    // The 6502 has a variety of addressing modes to access data in
    // memory, some of which are direct and some are indirect (like
    // pointers in C++). Each opcode contains information about which
    // addressing mode should be employed to facilitate the
    // instruction, in regards to where it reads/writes the data it
    // uses. The address mode changes the number of bytes that
    // makes up the full instruction, so we implement addressing
    // before executing the instruction, to make sure the program
    // counter is at the correct location, the instruction is
    // primed with the addresses it needs, and the number of clock
    // cycles the instruction requires is calculated. These functions
    // may adjust the number of cycles required depending upon where
    // and how the memory is accessed, so they return the required
    // adjustment.

    uint8_t IMP(); uint8_t IMM();
    uint8_t ZP0(); uint8_t ZPX();
    uint8_t ZPY(); uint8_t REL();
    uint8_t ABS(); uint8_t ABX();
    uint8_t ABY(); uint8_t IND();
    uint8_t IZX(); uint8_t IZY();

    // Opcodes ======================================================
    // There are 56 "legitimate" opcodes provided by the 6502 CPU. I
    // have not modelled "unofficial" opcodes. As each opcode is
    // defined by 1 byte, there are potentially 256 possible codes.
    // Codes are not used in a "switch case" style on a processor,
    // instead they are repsonisble for switching individual parts of
    // CPU circuits on and off. The opcodes listed here are official,
    // meaning that the functionality of the chip when provided with
    // these codes is as the developers intended it to be. Unofficial
    // codes will of course also influence the CPU circuitry in
    // interesting ways, and can be exploited to gain additional
    // functionality!
    //
    // These functions return 0 normally, but some are capable of
    // requiring more clock cycles when executed under certain
    // conditions combined with certain addressing modes. If that is
    // the case, they return 1.
    //
    // I have included detailed explanations of each function in
    // the class implementation file. Note they are listed in
    // alphabetical order here for ease of finding.

    uint8_t ADC();	uint8_t AND();	uint8_t ASL();	uint8_t BCC();
    uint8_t BCS();	uint8_t BEQ();	uint8_t BIT();	uint8_t BMI();
    uint8_t BNE();	uint8_t BPL();	uint8_t BRK();	uint8_t BVC();
    uint8_t BVS();	uint8_t CLC();	uint8_t CLD();	uint8_t CLI();
    uint8_t CLV();	uint8_t CMP();	uint8_t CPX();	uint8_t CPY();
    uint8_t DEC();	uint8_t DEX();	uint8_t DEY();	uint8_t EOR();
    uint8_t INC();	uint8_t INX();	uint8_t INY();	uint8_t JMP();
    uint8_t JSR();	uint8_t LDA();	uint8_t LDX();	uint8_t LDY();
    uint8_t LSR();	uint8_t NOP();	uint8_t ORA();	uint8_t PHA();
    uint8_t PHP();	uint8_t PLA();	uint8_t PLP();	uint8_t ROL();
    uint8_t ROR();	uint8_t RTI();	uint8_t RTS();	uint8_t SBC();
    uint8_t SEC();	uint8_t SED();	uint8_t SEI();	uint8_t STA();
    uint8_t STX();	uint8_t STY();	uint8_t TAX();	uint8_t TAY();
    uint8_t TSX();	uint8_t TXA();	uint8_t TXS();	uint8_t TYA();

    // I capture all "unofficial" opcodes with this function. It is
    // functionally identical to a NOP
    uint8_t XXX();

    // Indicates the current instruction has completed by returning true. This is
    // a utility function to enable "step-by-step" execution, without manually
    // clocking every cycle
    bool complete() const { return remainingCyclesForInstruction() == 0; }

    uint8_t remainingCyclesForInstruction() const { return _cycles; }

    void reset();
    void irq();
    void nmi();

    void clock(); ///< Executes one clock tick
    uint32_t clock_ticks = 0; // A global accumulation of the number of clocks

    const Registers &registers() const { return _registers; }
          Registers &registers()       { return _registers; }

    auto disassemble(addressType start, addressType stop) const -> disassemblyType;

    static constexpr uint16_t NMIAddress = 0xFFFA;
    static constexpr uint16_t ResetJumpStartAddress = 0xFFFC;
    static constexpr uint16_t IRQAddress = 0xFFFE;

    InstructionExecutor &operator =(const InstructionExecutor &) = delete;
    InstructionExecutor &operator =(InstructionExecutor &&) = delete;
protected:
    uint8_t  _fetched = 0x00; // Represents the working input value to the ALU
    uint16_t _temp = 0x0000; // A convenience variable used everywhere
    uint16_t _addr_abs = 0x0000; // All used memory addresses end up in here
    uint16_t _addr_rel = 0x0000; // Represents absolute address following a branch
    uint8_t  _opcode = 0x00; // Is the instruction byte
    uint8_t  _cycles = 0; // Counts how many cycles the instruction has remaining
    std::vector<INSTRUCTION> _lookup;
    Registers    &_registers;
    readDelegate  _read_delegate;
    writeDelegate _write_delegate;
    registerValueChangedDelegate _a_changed;
    registerValueChangedDelegate _x_changed;
    registerValueChangedDelegate _y_changed;
    registerValueChangedDelegate _stack_pointer_changed;
    addressValueChangedDelegate  _program_counter_changed;
    addressValueChangedDelegate  _status_changed;

    struct BCDResult {
        uint8_t sum;
        bool    hi_nybble_carry;
    };

    // The read location of data can come from two sources, a memory address, or
    // its immediately available as part of the instruction. This function decides
    // depending on address mode of instruction byte
    uint8_t fetch();

    uint8_t read(addressType address, bool read_only = false);
    void    write(addressType address, uint8_t data);

    // Convenience functions to access status register
    uint8_t GetFlag(FLAGS6502 f) const { return _registers.GetFlag(f); }
    void    SetFlag(FLAGS6502 f, bool v) { _registers.SetFlag(f, v); }

    BCDResult addBCD(uint8_t left, uint8_t right);
    uint8_t   complement(uint8_t input, bool decimal_mode) const { return (decimal_mode) ? 0x99 - input :
                                                                                           0xFF ^ input; }
};

#endif // INSTRUCTIONEXECUTOR_HPP
