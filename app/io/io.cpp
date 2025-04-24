#include "io/io.hpp"
#include "io/SRecord/QSRecordStream.hpp"
#include "io/SimpleHex/QSimpleHexStream.hpp"
#include <iterator>
#include <algorithm>
#include <optional>
#include <cctype>
#include <utility>
#include <QFile>

using ReadMemoryBlockFunctionPtr_t =  OptionalProgram (*)(QIODevice *);
using FileTypeTable_t = std::map<QString, ReadMemoryBlockFunctionPtr_t>;

namespace
{

QString SuffixOf(const QString &s)
{
    auto position = s.lastIndexOf('.');

    return (position == -1 ) ? QString() : s.mid( position );
}

OptionalProgram ReturnOnlyError(QIODevice *device)
{
    Q_UNUSED(device)

    return {};
}

OptionalProgram ReadPRGFrom(QIODevice *device)
{
    // ASSUME: 16-bit little-endian address first, with the rest of the bytes
    //         following.
    QByteArray temp_data = device->read(2);

    if ( temp_data.size() != 2 )
        return std::nullopt;

    uint32_t address = temp_data[0] | (temp_data[1] << 8);

    temp_data = device->readAll();

    if ( temp_data.isEmpty() )
        return std::nullopt;

    return { Program{ address, MemoryBlocks{
                MemoryBlock{ address, Bytes{ temp_data.begin(), temp_data.end() } }
                                           }
                    }
           };
}

static OptionalProgram ReadSimpleHexFrom(QIODevice *device)
{
    QSimpleHexStream hex_stream( device );
    OptionalMemoryBlocks blocks = hex_stream.readAll();

    if ( !blocks.has_value() )
        return std::nullopt;

    return Program{ std::move(blocks.value()) };
}

namespace SRecord_IO
{

inline bool SRecordFoundIn(const SRecords &records, SRecords::const_iterator iter)
{
    return iter != records.end();
}

inline size_t CountSRecordsOfType(const SRecords &records, const int type)
{
    return std::count_if(records.begin(), records.end(), [&](const SRecord &iCurrentRecord) { return iCurrentRecord.type == type; });
}

inline SRecords::const_iterator FindSRecordOfType(const SRecords &records, const int record_type)
{
    return find_if(records.begin(), records.end(), [&](const SRecord &iCurrentRecord) { return iCurrentRecord.type == record_type; } );
}

inline auto SRecordsOfType(const SRecords &records, const int type) -> std::pair<SRecords::const_iterator, SRecords::const_iterator>
{
    return std::equal_range(records.begin(), records.end(),
                            SRecord{ type },
                            [&](const SRecord &iCurrentRecord, const SRecord &compare_to)
                            {
                                return iCurrentRecord.type == compare_to.type;
                            }
                           );
}

OptionalSRecords ReadSRecords(QIODevice *device)
{
    QSRecordStream stream(device);
    OptionalSRecords records = stream.readAll();

    return records;
}

OptionalProgram ReadSRecordsFrom(QIODevice *device)
{
    OptionalSRecords records = ReadSRecords(device);

    if ( !records.has_value() )
        return std::nullopt;

    return { Program{ ToMemoryBlocks( records.value() ) } };
}

OptionalProgram ReadS19RecordsFrom(QIODevice *device)
{
    OptionalSRecords records = ReadSRecords(device);

    if (!records.has_value())
        return std::nullopt;

    // Sort records in increasing order of:
    // 1: type
    // 2: address
    std::stable_sort(records.value().begin(), records.value().end(),
              [](const SRecord &left, const SRecord &right)
              {
                  if (left.type < right.type)
                      return true;
                  return (left.address < right.address);
              });

    // Look for records S0, S1, S5 (optional), S9
    auto s0_count = CountSRecordsOfType(records.value(), 0);

    if (s0_count != 1)
        return std::nullopt;

    auto [s1_begin, s1_end] = SRecordsOfType(records.value(), 1);

    if (!SRecordFoundIn(records.value(), s1_begin))
        return std::nullopt;

    auto s9 = FindSRecordOfType(records.value(), 9);
    auto s9_count = CountSRecordsOfType(records.value(), 9);

    if (s9_count != 1)
        return std::nullopt;

    return { Program{ s9->address, ToMemoryBlocks(s1_begin, s1_end) } };
}

}

FileTypeTable_t FileTypeTable{
    { QStringLiteral(".prg"),  &ReadPRGFrom },
    { QStringLiteral(".shex"), &ReadSimpleHexFrom },
    { QStringLiteral(".srec"), &SRecord_IO::ReadSRecordsFrom },
    { QStringLiteral(".s19"),  &SRecord_IO::ReadS19RecordsFrom }
};

}

OptionalProgram ReadFromFile(QString filename)
{
    if ( !QFile::exists( filename ) )
        return std::nullopt;

    QFile file{ filename };

    if ( !file.open( QIODevice::ReadOnly ) )
        return std::nullopt;

    QString suffix = SuffixOf( filename );

    if ( FileTypeTable.count( suffix ) )
        return (FileTypeTable[ suffix ])( &file );
    else
        return ReturnOnlyError( &file );
}
