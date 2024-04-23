#ifndef IO_HPP
#define IO_HPP

#include "memory_block.hpp"

#include <QString>

using OptionalMemoryBlock = std::optional<MemoryBlock>;
using OptionalMemoryBlocks = std::optional<MemoryBlocks>;

struct Program
{
    explicit Program(uint32_t exec_address, MemoryBlocks &&memory) : begin_execution_address(exec_address), data(memory) { }
    explicit Program(MemoryBlocks &&memory) : data(memory) { }

    static constexpr uint32_t NoExecutionAddress = UINT32_MAX;

    uint32_t     begin_execution_address = NoExecutionAddress; // Use this value to mean "non-existent"
    MemoryBlocks data;

    bool hasValidExecutionAddress() const { return begin_execution_address != NoExecutionAddress; }
};

using OptionalProgram = std::optional<Program>;

OptionalProgram ReadFromFile(QString filename);

#endif // IO_HPP
