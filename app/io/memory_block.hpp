#pragma once

#include <optional>
#include <vector>
#include <utility>
#include <cstdint>


using Bytes = std::vector<uint8_t>;
using MemoryBlock = std::pair<int, Bytes>;
using MemoryBlocks = std::vector<MemoryBlock>;
