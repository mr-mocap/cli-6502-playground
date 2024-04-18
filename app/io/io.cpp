#include "io.hpp"
#include "QSRecordStream.hpp"
#include <iterator>
#include <algorithm>
#include <optional>
#include <cctype>
#include <utility>
#include <QFile>

using ReadMemoryBlockFunctionPtr_t =  std::optional<MemoryBlocks> (*)(QIODevice *);
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

std::optional<MemoryBlocks> ReturnOnlyError(QIODevice *device)
{
    Q_UNUSED(device)

    return {};
}

std::optional<MemoryBlocks> ReadPRGFrom(QIODevice *device)
{
    // ASSUME: 16-bit little-endian address first, with the rest of the bytes
    //         following.
    QByteArray temp_data = device->read(2);

    if ( temp_data.size() != 2 )
        return {};

    int address = temp_data[0] | (temp_data[1] << 8);

    temp_data = device->readAll();

    if ( temp_data.isEmpty() )
        return {};

    return MemoryBlocks{ MemoryBlock{ address, Bytes{ temp_data.begin(), temp_data.end() } } };
}

static constexpr std::string_view EatWhiteSpace(std::string_view data)
{
    while ( !data.empty() && std::isblank( data.front() ) )
        data.remove_prefix( 1 );
    return data;
}
static std::optional<MemoryBlock> ReadSimpleHexLine(std::string_view data)
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

std::optional<MemoryBlocks> ReadSimpleHexFrom(QIODevice *device)
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
    return { data };
}

namespace SRecord_IO
{

std::optional<MemoryBlocks> ReadSRecordFrom(QIODevice *device)
{
#if 0
    char buffer[514 + 1]; // 256 bytes of data plus first two bytes of any record. +1 is for the appended '\0'.
    Records local_records;

    while (true)
    {
        int line_length =  device->readLine( buffer, sizeof buffer );

        if ( line_length == -1 )
            break;

        // We have data in buffer
        int type = -2, byte_count = 0;
        std::string_view buffer_view(buffer, line_length);

        if ( buffer_view.empty() )
            return { };

        if ( !readRecordStart(buffer_view) )
            return { };

        if ( (type = readRecordType(buffer_view)) == -1 )
            return { };

        if ( (byte_count = readRecordByteCount(buffer_view)) == -1 )
            return { };

        if ( line_length < (byte_count * CharsPerByte) ) // Two hex chars per "byte", remember
            return { };

        OptionalRecord data = std::nullopt;

        switch (type)
        {
        case 0:
            data = readRecordType0( buffer_view.substr( 4, byte_count * 2) );
            break;
        case 1:
            data = readRecordType1( buffer_view.substr( 4, byte_count * 2) );
            break;
        case 2:
            data = readRecordType2( buffer_view.substr( 4, byte_count * 2) );
            break;
        case 3:
            data = readRecordType3( buffer_view.substr( 4, byte_count * 2) );
            break;
        case 4:
            data = readRecordType4( buffer_view.substr( 4, byte_count * 2) );
            break;
        case 5:
            data = readRecordType5( buffer_view.substr( 4, byte_count * 2) );
            break;
        case 6:
            data = readRecordType6( buffer_view.substr( 4, byte_count * 2) );
            break;
        case 7:
            data = readRecordType7( buffer_view.substr( 4, byte_count * 2) );
            break;
        case 8:
            data = readRecordType8( buffer_view.substr( 4, byte_count * 2) );
            break;
        case 9:
            data = readRecordType9( buffer_view.substr( 4, byte_count * 2) );
            break;
        default:
            break;
        }
        if ( data.has_value() )
            local_records.push_back( data.value() );
    }
#if 0
    if ( !local_records.empty() )
        records = std::move(local_records);
#endif
#else
    QSRecordStream stream(device);
    OptionalSRecords records = stream.readAll();

    if ( !records.has_value() )
        return MemoryBlocks();

    return { ToMemoryBlocks( records.value() ) };
#endif

}

}

FileTypeTable_t FileTypeTable{
    { QStringLiteral(".prg"), &ReadPRGFrom },
    { QStringLiteral(".shex"), &ReadSimpleHexFrom },
    { QStringLiteral(".srec"), &SRecord_IO::ReadSRecordFrom }
};

}

OptionalMemoryBlocks ReadFromFile(QString filename)
{
    if ( !QFile::exists( filename ) )
        return {};

    QFile file{ filename };

    if ( !file.open( QIODevice::ReadOnly ) )
        return {};

    QString suffix = SuffixOf( filename );

    if ( FileTypeTable.count( suffix ) )
        return (FileTypeTable[ suffix ])( &file );
    else
        return ReturnOnlyError( &file );
}
