#ifndef IO_HPP
#define IO_HPP

#include <QIODevice>
#include <optional>
#include <vector>
#include <map>
#include <string_view>

using Bytes = std::vector<uint8_t>;
using MemoryBlock = std::pair<int, Bytes>;
using MemoryBlocks = std::vector<MemoryBlock>;

std::optional<MemoryBlocks> ReadFromFile(QString filename);

#endif // IO_HPP
