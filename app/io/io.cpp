#include "io.hpp"
#include "QSRecordStream.hpp"
#include <iterator>
#include <algorithm>
#include <optional>
#include <cctype>
#include <utility>
#include <QFile>

using ReadMemoryBlockFunctionPtr_t =  OptionalProgram (*)(QIODevice *);
using FileTypeTable_t = std::map<QString, ReadMemoryBlockFunctionPtr_t>;


static inline bool BeginsWith8BitHexValue(std::string_view data)
{
    return ( data.size() >= 2 ) && std::isxdigit( data[0] ) && std::isxdigit( data[1] );
}

static constexpr inline bool IsDecimalDigit(char digit)
{
    return (digit >= '0') && (digit <= '9');
}

static constexpr inline bool IsHexDigit(char digit)
{
    return (digit >= 'A') && (digit <= 'F');
}

static int HexDigitsBigEndianToDecimal(std::string_view hex_digits_in_ascii)
{
    int value = 0;

    if ( hex_digits_in_ascii.empty() )
        return -1;

    for (char iCurrentHexDigit : hex_digits_in_ascii)
    {
        if ( IsDecimalDigit(iCurrentHexDigit) )
            iCurrentHexDigit -= '0';
        else if ( IsHexDigit(iCurrentHexDigit) )
            iCurrentHexDigit = iCurrentHexDigit - 'A' + 10;
        else
            return -1;

        value = (value << 4) | (iCurrentHexDigit & 0x0F);
    }
    return value;
}

static inline int Read8BitHexValue(std::string_view data)
{
    return HexDigitsBigEndianToDecimal( data );
}

static inline int Read16BitHexValue(std::string_view data)
{
    return HexDigitsBigEndianToDecimal( data );
}

static inline int Read24BitHexValue(std::string_view data)
{
    return HexDigitsBigEndianToDecimal( data );
}

static inline int Read32BitHexValue(std::string_view data)
{
    return HexDigitsBigEndianToDecimal( data );
}

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

static constexpr std::string_view EatWhiteSpace(std::string_view data)
{
    while ( !data.empty() && std::isblank( data.front() ) )
        data.remove_prefix( 1 );
    return data;
}
static OptionalMemoryBlock ReadSimpleHexLine(std::string_view data)
{
    if ( data.size() < 4 )
        return std::nullopt;

    std::string_view address = data.substr( 0, 4 );
    int value = Read16BitHexValue( address );

    // Expect ':'
    if ( data[4] != ':' )
        return std::nullopt;

    // Remove the address and ':'
    data = data.substr( 5 );

    std::vector<uint8_t> bytes;

    bytes.reserve(16); // Expect no more than 16 values (but there could be)

    while ( !(data = EatWhiteSpace( data )).empty() && BeginsWith8BitHexValue( data ) )
    {
        int byte_value = Read8BitHexValue( data );

        if ( byte_value == -1 )
            break;

        bytes.push_back( static_cast<uint8_t>(byte_value) );
    }
    return { std::make_pair( value, bytes ) };
}

static OptionalProgram ReadSimpleHexFrom(QIODevice *device)
{
    // We expect lines of the form: XXXX:( XX)*
    // A 16-bit address followed by a colon folowed by one or more 2-digit
    // hex chars (a byte value).
    MemoryBlocks data;

    while (true)
    {
        QByteArray line = device->readLine();

        if ( line.isEmpty() )
            break;

        auto input_data = ReadSimpleHexLine( std::string_view( line.data(), line.size() ) );

        if ( !input_data.has_value() )
            break;

        data.emplace_back( input_data.value() );
    }
    return { Program{ std::move(data) } };
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
    auto s0 = FindSRecordOfType(records.value(), 0);
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
