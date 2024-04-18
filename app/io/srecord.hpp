#pragma once

#include "memory_block.hpp"
#include <cctype>
#include <string_view>
#include <array>

class QIODevice;

struct SRecord
{
    // These are in bytes in memory and not the representation in the stream
    static const int AddressLength16Bit = 2;
    static const int AddressLength24Bit = 3;
    static const int AddressLength32Bit = 4;

    // Units are in bytes
    static const int StartOfRecordSize = 1; // 'S'
    static const int RecordTypeSize = 1;    // '0' - '9'
    static const int ByteCountSize = 1;
    static const int MaxAddressSize = AddressLength32Bit;
    static const int MaxDataSize = 256;
    static const int ChecksumSize = 1;

    // Offsets in the raw record line
    static const int StartOfRecordOffset = 0;
    static const int RecordTypeOffset = 1;
    static const int ByteCountOffset = 2;
    static const int AddressOffset = 4;
    static const int DataOffsetWith16BitAddress = AddressOffset + (AddressLength16Bit * 2);
    static const int DataOffsetWith24BitAddress = AddressOffset + (AddressLength24Bit * 2);
    static const int DataOffsetWith32BitAddress = AddressOffset + (AddressLength32Bit * 2);

    static const int MaxRecordSize = StartOfRecordSize + RecordTypeSize + (ByteCountSize + MaxAddressSize + MaxDataSize + ChecksumSize) * 2;

    using Buffer = std::array<char, MaxRecordSize + 1>; // This includes room for a '\0'

    SRecord(int t, uint32_t a, const Bytes &b, uint8_t c) : type(t), address(a), data(b), checksum(c) { }
    SRecord(int t, uint32_t a, const Bytes &b) : type(t), address(a), data(b) { }

    int      type = 0;
    uint32_t address = 0;
    Bytes    data;
    uint8_t  checksum = 0;
};

using SRecords = std::vector<SRecord>;
using OptionalSRecord = std::optional<SRecord>;
using OptionalSRecords = std::optional<SRecords>;

inline MemoryBlock ToMemoryBlock(const SRecord &record)
{
   return std::make_pair( record.address, record.data );
}

MemoryBlocks ToMemoryBlocks(const SRecords &records);
