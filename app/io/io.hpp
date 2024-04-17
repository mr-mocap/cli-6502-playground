#ifndef IO_HPP
#define IO_HPP

#include "memory_block.hpp"

#include <QString>

std::optional<MemoryBlocks> ReadFromFile(QString filename);

#endif // IO_HPP
