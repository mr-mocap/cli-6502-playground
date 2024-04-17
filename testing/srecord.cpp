#include "srecord.hpp"
#include <QIODevice>
#include <algorithm>
#include <array>
#include <charconv>


MemoryBlocks ToMemoryBlocks(const SRecords &records)
{
    MemoryBlocks memory_blocks;

    memory_blocks.reserve( records.size() );
    for (const SRecord &iCurrentRecord : records)
    {
        memory_blocks.push_back( ToMemoryBlock(iCurrentRecord) );
    }
    return memory_blocks;
}
