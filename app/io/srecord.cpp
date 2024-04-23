#include "srecord.hpp"
#include <QIODevice>
#include <algorithm>
#include <array>
#include <charconv>


MemoryBlocks ToMemoryBlocks(const SRecords &records)
{
    return ToMemoryBlocks( records.begin(), records.end() );
}

MemoryBlocks ToMemoryBlocks(SRecords::const_iterator begin_range, SRecords::const_iterator end_range)
{
    MemoryBlocks memory_blocks;

    memory_blocks.reserve( std::distance(begin_range, end_range) );
    std::transform( begin_range, end_range,
                    std::back_inserter(memory_blocks),
                    [](const SRecord &iCurrentRecord)
                    {
                        return ToMemoryBlock(iCurrentRecord);
                    }
                  );
    return memory_blocks;
}
