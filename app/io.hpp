#ifndef IO_HPP
#define IO_HPP

#include <QIODevice>
#include <optional>
#include <vector>
#include <map>
#include <string_view>

using Bytes = std::vector<uint8_t>;
using MemoryBlock = std::pair<int, Bytes>;

std::optional<MemoryBlock> ReadFromFile(QString filename);

#endif // IO_HPP
