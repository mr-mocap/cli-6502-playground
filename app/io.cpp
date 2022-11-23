#include "io.hpp"
#include <iterator>

namespace
{

static constexpr int CharsPerByte = 2;

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

static IOSRecord::Bytes ReadData(std::string_view data)
{
    std::vector<char> rest;

    rest.reserve( data.size() / 2 );

    for ( ; !data.empty(); data.remove_prefix( 1 * CharsPerByte ) )
    {
        if (int value = Read8BitHexValue( data.substr( 0, 2 ) ); value != -1)
            rest.push_back( static_cast<char>(value) );
    }

    return rest;
}

}

void IOSRecord::ReadInputFrom(QIODevice *device)
{
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
            return;

        if ( !readRecordStart(buffer_view) )
            return;

        if ( (type = readRecordType(buffer_view)) == -1 )
            return;

        if ( (byte_count = readRecordByteCount(buffer_view)) == -1 )
            return;

        if ( line_length < (byte_count * CharsPerByte) ) // Two hex chars per "byte", remember
            return;

        RecordData data = std::nullopt;

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

    if ( !local_records.empty() )
        records = std::move(local_records);
}

bool IOSRecord::readRecordStart(const std::string_view data)
{
    return data.front() == 'S';
}

int IOSRecord::readRecordType(const std::string_view data)
{
    return IsDecimalDigit( data[1] ) ? data[1] - '0' : -1;
}

int IOSRecord::readRecordByteCount(const std::string_view data)
{
    return Read8BitHexValue( data.substr( 2, 2 ) );
}

IOSRecord::RecordData IOSRecord::readRecordType0(std::string_view data)
{
    // Check if the Address part is 0
    if ( data.size() >= (3 * CharsPerByte) )
    {
        if (int address = Read16BitHexValue( data.substr(0, 2 * CharsPerByte) ); address == 0)
        {
            data.remove_prefix( 2 * CharsPerByte );
            if ( data.size() >= ( 1 * CharsPerByte) )
            {
                std::string_view checksum_bytes( data.substr( data.size() - (1 * CharsPerByte) ) );
                int checksum = Read8BitHexValue( checksum_bytes );

                // Remove checksum
                data.remove_suffix( 1 * CharsPerByte );

                return Record(address, ReadData(data), checksum);
            }
        }
    }

    return {};
}

IOSRecord::RecordData IOSRecord::readRecordType1(std::string_view data)
{
    if ( data.size() >= (3 * CharsPerByte) )
    {
        if (int address = Read16BitHexValue( data.substr(0, 2 * CharsPerByte) ); address != -1)
        {
            data.remove_prefix( 2 * CharsPerByte );
            if ( data.size() >= ( 1 * CharsPerByte) )
            {
                std::string_view checksum_bytes( data.substr( data.size() - (1 * CharsPerByte) ) );
                int checksum = Read8BitHexValue( checksum_bytes );

                // Remove checksum
                data.remove_suffix( 1 * CharsPerByte );

                return Record(address, ReadData(data), checksum);
            }
        }
    }

    return {};
}

IOSRecord::RecordData IOSRecord::readRecordType2(std::string_view data)
{
    if ( data.size() >= (4 * CharsPerByte) )
    {
        if (int address = Read24BitHexValue( data.substr(0, 3 * CharsPerByte) ); address != -1)
        {
            data.remove_prefix( 3 * CharsPerByte );
            if ( data.size() >= ( 1 * CharsPerByte) )
            {
                std::string_view checksum_bytes( data.substr( data.size() - (1 * CharsPerByte) ) );
                int checksum = Read8BitHexValue( checksum_bytes );

                // Remove checksum
                data.remove_suffix( 1 * CharsPerByte );

                return Record(address, ReadData(data), checksum);
            }
        }
    }

    return {};
}

IOSRecord::RecordData IOSRecord::readRecordType3(std::string_view data)
{
    if ( data.size() >= (5 * CharsPerByte) )
    {
        if (int address = Read32BitHexValue( data.substr(0, 4 * CharsPerByte) ); address != -1)
        {
            data.remove_prefix( 4 * CharsPerByte );
            if ( data.size() >= ( 1 * CharsPerByte) )
            {
                std::string_view checksum_bytes( data.substr( data.size() - (1 * CharsPerByte) ) );
                int checksum = Read8BitHexValue( checksum_bytes );

                // Remove checksum
                data.remove_suffix( 1 * CharsPerByte );

                return Record(address, ReadData(data), checksum);
            }
        }
    }

    return {};
}

IOSRecord::RecordData IOSRecord::readRecordType5(std::string_view data)
{
    if ( data.size() >= (3 * CharsPerByte) )
    {
        if (int address = Read16BitHexValue( data.substr(0, 2 * CharsPerByte) ); address != -1)
        {
            data.remove_prefix( 2 * CharsPerByte );
            if ( data.size() >= ( 1 * CharsPerByte) )
            {
                std::string_view checksum_bytes( data.substr( data.size() - (1 * CharsPerByte) ) );
                int checksum = Read8BitHexValue( checksum_bytes );

                // Let's don't read the data because it really shouldn't be there
                return Record(address, Bytes(), checksum);
            }
        }
    }

    return {};
}

IOSRecord::RecordData IOSRecord::readRecordType6(std::string_view data)
{
    if ( data.size() >= (4 * CharsPerByte) )
    {
        if (int address = Read24BitHexValue( data.substr(0, 3 * CharsPerByte) ); address != -1)
        {
            data.remove_prefix( 3 * CharsPerByte );
            if ( data.size() >= ( 1 * CharsPerByte) )
            {
                std::string_view checksum_bytes( data.substr( data.size() - (1 * CharsPerByte) ) );
                int checksum = Read8BitHexValue( checksum_bytes );

                // Let's don't read the data because it really shouldn't be there
                return Record(address, Bytes(), checksum);
            }
        }
    }

    return {};
}

IOSRecord::RecordData IOSRecord::readRecordType7(std::string_view data)
{
    if ( data.size() >= (5 * CharsPerByte) )
    {
        if (int address = Read32BitHexValue( data.substr(0, 4 * CharsPerByte) ); address != -1)
        {
            data.remove_prefix( 4 * CharsPerByte );
            if ( data.size() >= ( 1 * CharsPerByte) )
            {
                std::string_view checksum_bytes( data.substr( data.size() - (1 * CharsPerByte) ) );
                int checksum = Read8BitHexValue( checksum_bytes );

                // Let's don't read the data because it really shouldn't be there
                return Record(address, Bytes(), checksum);
            }
        }
    }

    return {};
}

IOSRecord::RecordData IOSRecord::readRecordType8(std::string_view data)
{
    if ( data.size() >= (4 * CharsPerByte) )
    {
        if (int address = Read24BitHexValue( data.substr(0, 3 * CharsPerByte) ); address != -1)
        {
            data.remove_prefix( 3 * CharsPerByte );
            if ( data.size() >= ( 1 * CharsPerByte) )
            {
                std::string_view checksum_bytes( data.substr( data.size() - (1 * CharsPerByte) ) );
                int checksum = Read8BitHexValue( checksum_bytes );

                // Let's don't read the data because it really shouldn't be there
                return Record(address, Bytes(), checksum);
            }
        }
    }

    return {};
}

IOSRecord::RecordData IOSRecord::readRecordType9(std::string_view data)
{
    if ( data.size() >= (3 * CharsPerByte) )
    {
        if (int address = Read16BitHexValue( data.substr(0, 2 * CharsPerByte) ); address != -1)
        {
            data.remove_prefix( 2 * CharsPerByte );
            if ( data.size() >= ( 1 * CharsPerByte) )
            {
                std::string_view checksum_bytes( data.substr( data.size() - (1 * CharsPerByte) ) );
                int checksum = Read8BitHexValue( checksum_bytes );

                // Let's don't read the data because it really shouldn't be there
                return Record(address, Bytes(), checksum);
            }
        }
    }

    return {};
}
