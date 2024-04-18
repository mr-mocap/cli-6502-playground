#ifndef IO_HPP
#define IO_HPP

#include "memory_block.hpp"

#include <QString>

using OptionalMemoryBlocks = std::optional<MemoryBlocks>;

OptionalMemoryBlocks ReadFromFile(QString filename);

#endif // IO_HPP
