#include "QSRecordStream.hpp"
#include "utilities/StringConversions.hpp"
#include <QIODevice>
#include <cstdio>
#include <algorithm>

const int CharsPerByte = 2;

static Bytes ReadData(std::string_view data)
{
    Bytes rest;

    rest.reserve( data.size() );

    for ( ; !data.empty(); data.remove_prefix( 1 * CharsPerByte ) )
    {
        if (int value = Read8BitHexValue( data.substr( 0, 2 ) ); value != -1)
            rest.push_back( static_cast<char>(value) );
    }

    return rest;
}

static uint8_t CalculateChecksumFromHexData(std::string_view hex_data)
{
    assert( !hex_data.empty() );
    assert( hex_data.size() % 2 == 0 );
    assert( std::all_of( hex_data.begin(), hex_data.end(), [](unsigned char c) { return std::isxdigit(c); } ) );

    unsigned int sum = 0;

    while ( !hex_data.empty() )
    {
        int byte_value = Read8BitHexValue( hex_data.substr(0, 2) );

        sum += static_cast<unsigned int>(byte_value);
        hex_data.remove_prefix(2);
    }

    // Take the least significant byte of the one's complement of the sum
    return 0xFF - (sum & 0xFF);
}

constexpr inline bool SRecordByteCountIsValid(const int count)
{
    return count >= 3 && count <= 255;
}

inline bool ReadSRecordStart(const std::string_view data)
{
    return data.front() == 'S';
}

inline int ReadSRecordType(const std::string_view data)
{
    return std::isdigit( data[1] ) ? data[1] - '0' : -1;
}

inline int ReadSRecordByteCount(const std::string_view data)
{
    return Read8BitHexValue( data.substr( 2, 2 ) );
}

inline std::string_view Read16BitAddress(std::string_view data)
{
    return (data.length() >= 8) ? data.substr(4, 2 * CharsPerByte) : std::string_view();
}

inline std::string_view Read24BitAddress(std::string_view data)
{
    return (data.length() >= 10) ? data.substr(4, 3 * CharsPerByte) : std::string_view();
}

inline std::string_view Read32BitAddress(std::string_view data)
{
    return (data.length() >= 12) ? data.substr(4, 4 * CharsPerByte) : std::string_view();
}

static std::string_view ReadDataPartThatRequiresPreceedingDataChars(std::string_view data, size_t string_bytes_preceeding_data_part)
{
    if (data.length() < string_bytes_preceeding_data_part)
        return { }; // String too short to contain data segment

    // Remove 'S', record type, byte count, and address
    data.remove_prefix(string_bytes_preceeding_data_part);

    // Now we are left with just the data and checksum segments.

    // Assume: checksum is always the last 2 hex digits
    if (data.length() >= 2)
        data.remove_suffix(2); // Remove that suffix!

    return data;
}

inline std::string_view ReadDataPartFor16BitAddressRecord(std::string_view data)
{
    return ReadDataPartThatRequiresPreceedingDataChars(data, 8);
}

inline std::string_view ReadDataPartFor24BitAddressRecord(std::string_view data)
{
    return ReadDataPartThatRequiresPreceedingDataChars(data, 10);
}

inline std::string_view ReadDataPartFor32BitAddressRecord(std::string_view data)
{
    return ReadDataPartThatRequiresPreceedingDataChars(data, 12);
}

inline std::string_view ReadChecksumPart(std::string_view data)
{
    if (data.length() < 2)
        return { };

    data.remove_prefix( data.length() - 2 );
    return data;
}

static OptionalSRecord ReadSRecordType0(std::string_view data)
{
    if ( !ReadSRecordStart(data) )
        return { };

    {
        int type = ReadSRecordType(data);

        if ( type == -1 )
            return { };
        if ( type != 0 )
            return { };
    }

    int byte_count = 0;

    // byte_count is the number of bytes FOLLOWING this field and to the end of the record
    if ( (byte_count = ReadSRecordByteCount(data)) == -1 )
        return { };

    if ( !SRecordByteCountIsValid(byte_count) )
        return { };

    std::string_view address_string = Read16BitAddress(data);

    if ( address_string.empty() )
        return { }; // It either didn't exist or was too short

    // Check if the Address part is 0
    int address = Read16BitHexValue( address_string );

    if ( address == -1 )
        return { }; // Not a hex number

    if ( address != 0 )
        return { }; // Invalid address

    // address must be 0

    std::string_view data_part = ReadDataPartFor16BitAddressRecord(data);

    if ( data_part.empty() )
        return { };

    if ( data_part.length() % 2 != 0 )
        return { }; // We need two hex digits per byte

    int bytes_in_data_part = data_part.length() / 2;

    // Check for byte_count matching the data length (minus the address (2-bytes) and checksum (1 byte) )
    if ( bytes_in_data_part != (byte_count - 3) )
        return { };

    std::string_view checksum_part = ReadChecksumPart( data );

    if ( checksum_part.empty() )
        return { }; // It wasn't there

    if ( checksum_part.length() != 2 )
        return { }; // Incorrect checksum size

    int checksum_in_record = Read8BitHexValue( checksum_part );

    if ( checksum_in_record == -1 )
        return { }; // Not a hex number

    if ( static_cast<uint8_t>(checksum_in_record) != CalculateChecksumFromHexData(data.substr(2, data.size() - 4)) ) // strip first 2 & last 2 chars ('S0' & checksum)
        return { }; // Checksum doesn't match

    return SRecord(0, address, ReadData(data_part), checksum_in_record);
}

static OptionalSRecord ReadSRecordType1(std::string_view data)
{
    if ( !ReadSRecordStart(data) )
        return { };

    {
        int type = ReadSRecordType(data);

        if ( type == -1 )
            return { };
        if ( type != 1 )
            return { };
    }

    int byte_count = 0;

    // byte_count is the number of bytes FOLLOWING this field and to the end of the record
    if ( (byte_count = ReadSRecordByteCount(data)) == -1 )
        return { };

    if ( !SRecordByteCountIsValid(byte_count) )
        return { };

    std::string_view address_string = Read16BitAddress(data);

    if ( address_string.empty() )
        return { }; // It either didn't exist or was too short

    int address = Read16BitHexValue( address_string );

    if ( address == -1 )
        return { }; // Not a hex number

    std::string_view data_part = ReadDataPartFor16BitAddressRecord(data);

    if ( data_part.empty() )
        return { };

    if ( data_part.length() % 2 != 0 )
        return { }; // We need two hex digits per byte

    int bytes_in_data_part = data_part.length() / 2;

    // Check for byte_count matching the data length (minus the address (2-bytes) and checksum (1 byte) )
    if ( bytes_in_data_part != (byte_count - 3) )
        return { };

    std::string_view checksum_part = ReadChecksumPart( data );

    if ( checksum_part.empty() )
        return { }; // It wasn't there

    if ( checksum_part.length() != 2 )
        return { }; // Incorrect checksum size

    int checksum_in_record = Read8BitHexValue( checksum_part );

    if ( checksum_in_record == -1 )
        return { }; // Not a hex number

    if ( static_cast<uint8_t>(checksum_in_record) != CalculateChecksumFromHexData(data.substr(2, data.size() - 4)) ) // strip first 2 & last 2 chars ('S0' & checksum)
        return { }; // Checksum doesn't match

    return SRecord(1, address, ReadData(data_part), checksum_in_record);
}

static OptionalSRecord ReadSRecordType2(std::string_view data)
{
    if ( !ReadSRecordStart(data) )
        return { };

    {
        int type = ReadSRecordType(data);

        if ( type == -1 )
            return { };
        if ( type != 2 )
            return { };
    }

    int byte_count = 0;

    // byte_count is the number of bytes FOLLOWING this field and to the end of the record
    if ( (byte_count = ReadSRecordByteCount(data)) == -1 )
        return { };

    if ( !SRecordByteCountIsValid(byte_count) )
        return { };

    std::string_view address_string = Read24BitAddress(data);

    if ( address_string.empty() )
        return { }; // It either didn't exist or was too short

    int address = Read24BitHexValue( address_string );

    if ( address == -1 )
        return { }; // Not a hex number

    std::string_view data_part = ReadDataPartFor24BitAddressRecord(data);

    if ( data_part.empty() )
        return { };

    if ( data_part.length() % 2 != 0 )
        return { }; // We need two hex digits per byte

    int bytes_in_data_part = data_part.length() / 2;

    // Check for byte_count matching the data length (minus the address (3-bytes) and checksum (1 byte) )
    if ( bytes_in_data_part != (byte_count - 4) )
        return { };

    std::string_view checksum_part = ReadChecksumPart( data );

    if ( checksum_part.empty() )
        return { }; // It wasn't there

    if ( checksum_part.length() != 2 )
        return { }; // Incorrect checksum size

    int checksum_in_record = Read8BitHexValue( checksum_part );

    if ( checksum_in_record == -1 )
        return { }; // Not a hex number

    if ( static_cast<uint8_t>(checksum_in_record) != CalculateChecksumFromHexData(data.substr(2, data.size() - 4)) ) // strip first 2 & last 2 chars ('S0' & checksum)
        return { }; // Checksum doesn't match

    return SRecord(2, address, ReadData(data_part), checksum_in_record);
}

static OptionalSRecord ReadSRecordType3(std::string_view data)
{
    if ( !ReadSRecordStart(data) )
        return { };

    {
        int type = ReadSRecordType(data);

        if ( type == -1 )
            return { };
        if ( type != 3 )
            return { };
    }

    int byte_count = 0;

    // byte_count is the number of bytes FOLLOWING this field and to the end of the record
    if ( (byte_count = ReadSRecordByteCount(data)) == -1 )
        return { };

    if ( !SRecordByteCountIsValid(byte_count) )
        return { };

    std::string_view address_string = Read32BitAddress(data);

    if ( address_string.empty() )
        return { }; // It either didn't exist or was too short

    int address = Read32BitHexValue( address_string );

    if ( address == -1 )
        return { }; // Not a hex number

    std::string_view data_part = ReadDataPartFor32BitAddressRecord(data);

    if ( data_part.empty() )
        return { };

    if ( data_part.length() % 2 != 0 )
        return { }; // We need two hex digits per byte

    int bytes_in_data_part = data_part.length() / 2;

    // Check for byte_count matching the data length (minus the address (3-bytes) and checksum (1 byte) )
    if ( bytes_in_data_part != (byte_count - 5) )
        return { };

    std::string_view checksum_part = ReadChecksumPart( data );

    if ( checksum_part.empty() )
        return { }; // It wasn't there

    if ( checksum_part.length() != 2 )
        return { }; // Incorrect checksum size

    int checksum_in_record = Read8BitHexValue( checksum_part );

    if ( checksum_in_record == -1 )
        return { }; // Not a hex number

    if ( static_cast<uint8_t>(checksum_in_record) != CalculateChecksumFromHexData(data.substr(2, data.size() - 4)) ) // strip first 2 & last 2 chars ('S0' & checksum)
        return { }; // Checksum doesn't match

    return SRecord(3, address, ReadData(data_part), checksum_in_record);
}

static OptionalSRecord ReadSRecordType4(std::string_view data)
{
    Q_UNUSED(data);

    // RESERVED by the specification
    return {};
}

static OptionalSRecord ReadSRecordType5(std::string_view data)
{
    if ( !ReadSRecordStart(data) )
        return { };

    {
        int type = ReadSRecordType(data);

        if ( type == -1 )
            return { };
        if ( type != 5 )
            return { };
    }

    int byte_count = 0;

    // byte_count is the number of bytes FOLLOWING this field and to the end of the record
    if ( (byte_count = ReadSRecordByteCount(data)) == -1 )
        return { };

    if ( !SRecordByteCountIsValid(byte_count) )
        return { };

    std::string_view address_string = Read16BitAddress(data);

    if ( address_string.empty() )
        return { }; // It either didn't exist or was too short

    int address = Read16BitHexValue( address_string );

    if ( address == -1 )
        return { }; // Not a hex number

    std::string_view data_part = ReadDataPartFor16BitAddressRecord(data);

    // We must have NO DATA for this record type.
    // All our interesting information is in the address segment.
    if ( !data_part.empty() )
        return { };

    std::string_view checksum_part = ReadChecksumPart( data );

    if ( checksum_part.empty() )
        return { }; // It wasn't there

    if ( checksum_part.length() != 2 )
        return { }; // Incorrect checksum size

    int checksum_in_record = Read8BitHexValue( checksum_part );

    if ( checksum_in_record == -1 )
        return { }; // Not a hex number

    if ( static_cast<uint8_t>(checksum_in_record) != CalculateChecksumFromHexData(data.substr(2, data.size() - 4)) ) // strip first 2 & last 2 chars ('S0' & checksum)
        return { }; // Checksum doesn't match

    return SRecord(5, address, ReadData(data_part), checksum_in_record);
}

static OptionalSRecord ReadSRecordType6(std::string_view data)
{
    if ( !ReadSRecordStart(data) )
        return { };

    {
        int type = ReadSRecordType(data);

        if ( type == -1 )
            return { };
        if ( type != 6 )
            return { };
    }

    int byte_count = 0;

    // byte_count is the number of bytes FOLLOWING this field and to the end of the record
    if ( (byte_count = ReadSRecordByteCount(data)) == -1 )
        return { };

    if ( !SRecordByteCountIsValid(byte_count) )
        return { };

    std::string_view address_string = Read24BitAddress(data);

    if ( address_string.empty() )
        return { }; // It either didn't exist or was too short

    int address = Read24BitHexValue( address_string );

    if ( address == -1 )
        return { }; // Not a hex number

    std::string_view data_part = ReadDataPartFor24BitAddressRecord(data);

    // We must have NO DATA for this record type.
    // All our interesting information is in the address segment.
    if ( !data_part.empty() )
        return { };

    std::string_view checksum_part = ReadChecksumPart( data );

    if ( checksum_part.empty() )
        return { }; // It wasn't there

    if ( checksum_part.length() != 2 )
        return { }; // Incorrect checksum size

    int checksum_in_record = Read8BitHexValue( checksum_part );

    if ( checksum_in_record == -1 )
        return { }; // Not a hex number

    if ( static_cast<uint8_t>(checksum_in_record) != CalculateChecksumFromHexData(data.substr(2, data.size() - 4)) ) // strip first 2 & last 2 chars ('S0' & checksum)
        return { }; // Checksum doesn't match

    return SRecord(6, address, ReadData(data_part), checksum_in_record);
}

static OptionalSRecord ReadSRecordType7(std::string_view data)
{
    if ( !ReadSRecordStart(data) )
        return { };

    {
        int type = ReadSRecordType(data);

        if ( type == -1 )
            return { };
        if ( type != 7 )
            return { };
    }

    int byte_count = 0;

    // byte_count is the number of bytes FOLLOWING this field and to the end of the record
    if ( (byte_count = ReadSRecordByteCount(data)) == -1 )
        return { };

    if ( !SRecordByteCountIsValid(byte_count) )
        return { };

    std::string_view address_string = Read32BitAddress(data);

    if ( address_string.empty() )
        return { }; // It either didn't exist or was too short

    int address = Read32BitHexValue( address_string );

    if ( address == -1 )
        return { }; // Not a hex number

    std::string_view data_part = ReadDataPartFor32BitAddressRecord(data);

    // We must have NO DATA for this record type.
    // All our interesting information is in the address segment.
    if ( !data_part.empty() )
        return { };

    std::string_view checksum_part = ReadChecksumPart( data );

    if ( checksum_part.empty() )
        return { }; // It wasn't there

    if ( checksum_part.length() != 2 )
        return { }; // Incorrect checksum size

    int checksum_in_record = Read8BitHexValue( checksum_part );

    if ( checksum_in_record == -1 )
        return { }; // Not a hex number

    if ( static_cast<uint8_t>(checksum_in_record) != CalculateChecksumFromHexData(data.substr(2, data.size() - 4)) ) // strip first 2 & last 2 chars ('S0' & checksum)
        return { }; // Checksum doesn't match

    return SRecord(7, address, ReadData(data_part), checksum_in_record);
}

static OptionalSRecord ReadSRecordType8(std::string_view data)
{
    if ( !ReadSRecordStart(data) )
        return { };

    {
        int type = ReadSRecordType(data);

        if ( type == -1 )
            return { };
        if ( type != 8 )
            return { };
    }

    int byte_count = 0;

    // byte_count is the number of bytes FOLLOWING this field and to the end of the record
    if ( (byte_count = ReadSRecordByteCount(data)) == -1 )
        return { };

    if ( !SRecordByteCountIsValid(byte_count) )
        return { };

    std::string_view address_string = Read24BitAddress(data);

    if ( address_string.empty() )
        return { }; // It either didn't exist or was too short

    int address = Read24BitHexValue( address_string );

    if ( address == -1 )
        return { }; // Not a hex number

    std::string_view data_part = ReadDataPartFor24BitAddressRecord(data);

    // We must have NO DATA for this record type.
    // All our interesting information is in the address segment.
    if ( !data_part.empty() )
        return { };

    std::string_view checksum_part = ReadChecksumPart( data );

    if ( checksum_part.empty() )
        return { }; // It wasn't there

    if ( checksum_part.length() != 2 )
        return { }; // Incorrect checksum size

    int checksum_in_record = Read8BitHexValue( checksum_part );

    if ( checksum_in_record == -1 )
        return { }; // Not a hex number

    if ( static_cast<uint8_t>(checksum_in_record) != CalculateChecksumFromHexData(data.substr(2, data.size() - 4)) ) // strip first 2 & last 2 chars ('S0' & checksum)
        return { }; // Checksum doesn't match

    return SRecord(8, address, ReadData(data_part), checksum_in_record);
}

static OptionalSRecord ReadSRecordType9(std::string_view data)
{
    if ( !ReadSRecordStart(data) )
        return { };

    {
        int type = ReadSRecordType(data);

        if ( type == -1 )
            return { };
        if ( type != 9 )
            return { };
    }

    int byte_count = 0;

    // byte_count is the number of bytes FOLLOWING this field and to the end of the record
    if ( (byte_count = ReadSRecordByteCount(data)) == -1 )
        return { };

    if ( !SRecordByteCountIsValid(byte_count) )
        return { };

    std::string_view address_string = Read16BitAddress(data);

    if ( address_string.empty() )
        return { }; // It either didn't exist or was too short

    int address = Read16BitHexValue( address_string );

    if ( address == -1 )
        return { }; // Not a hex number

    std::string_view data_part = ReadDataPartFor16BitAddressRecord(data);

    // We must have NO DATA for this record type.
    // All our interesting information is in the address segment.
    if ( !data_part.empty() )
        return { };

    std::string_view checksum_part = ReadChecksumPart( data );

    if ( checksum_part.empty() )
        return { }; // It wasn't there

    if ( checksum_part.length() != 2 )
        return { }; // Incorrect checksum size

    int checksum_in_record = Read8BitHexValue( checksum_part );

    if ( checksum_in_record == -1 )
        return { }; // Not a hex number

    if ( static_cast<uint8_t>(checksum_in_record) != CalculateChecksumFromHexData(data.substr(2, data.size() - 4)) ) // strip first 2 & last 2 chars ('S0' & checksum)
        return { }; // Checksum doesn't match

    return SRecord(9, address, ReadData(data_part), checksum_in_record);
}

// Returns length written
static int WritePrimitiveRecord(SRecord::Buffer &buffer, const SRecord &record, const int bytes_in_address)
{
    const size_t bytes_in_byte_count = 1;
    const int    bytes_in_checksum = 1;
    const int    value_in_byte_count_field = bytes_in_address + record.data.size() + bytes_in_checksum;
    const int    bytes_in_checksum_calculation = bytes_in_byte_count + bytes_in_address + record.data.size();

    snprintf( buffer.data(), buffer.size(), "S%c%02X%04X",
              record.type + '0',
              value_in_byte_count_field, // Byte count (2-byte address + 1-byte checksum)
              record.address);

    for (size_t byte_number_in_data = 0; byte_number_in_data < record.data.size(); ++byte_number_in_data)
    {
        size_t buffer_index_from_byte_number = 2 + (bytes_in_byte_count * 2) + (bytes_in_address * 2) + byte_number_in_data * 2;

        // We use 3 bytes because it is two hex chars PLUS a '\n'
        snprintf( &buffer[ buffer_index_from_byte_number ], 3, "%02X", record.data[byte_number_in_data] );
    }
    uint8_t checksum = CalculateChecksumFromHexData( std::string_view( &buffer[2], bytes_in_checksum_calculation * 2 ) );
    int length_up_to_checksum = 2 + (bytes_in_checksum_calculation * 2);

    snprintf( &buffer[length_up_to_checksum], 3, "%02X", checksum);
    return length_up_to_checksum + 2;
}

static void WriteSRecord0(const SRecord &record, QIODevice *device)
{
    Q_ASSERT(device);
    Q_ASSERT( record.address <= 0x0000FFFF); // 16-bit address

    SRecord::Buffer buffer;

    int number_bytes_written_to_buffer = WritePrimitiveRecord(buffer, record, 2);

    device->write( buffer.data(), number_bytes_written_to_buffer );
}

static void WriteSRecord1(const SRecord &record, QIODevice *device)
{
    WriteSRecord0(record, device);
}

static void WriteSRecord2(const SRecord &record, QIODevice *device)
{
    Q_ASSERT(device);
    Q_ASSERT( record.address <= 0x00FFFFFF); // 24-bit address

    SRecord::Buffer buffer;

    int number_bytes_written_to_buffer = WritePrimitiveRecord(buffer, record, 3);

    device->write( buffer.data(), number_bytes_written_to_buffer );
}

static void WriteSRecord3(const SRecord &record, QIODevice *device)
{
    Q_ASSERT(device);
    Q_ASSERT( record.address <= 0xFFFFFFFF); // 32-bit address

    SRecord::Buffer buffer;

    int number_bytes_written_to_buffer = WritePrimitiveRecord(buffer, record, 4);

    device->write( buffer.data(), number_bytes_written_to_buffer );
}

static void WriteSRecord5(const SRecord &record, QIODevice *device)
{
    Q_ASSERT(device);
    Q_ASSERT( record.address <= 0x0000FFFF); // 16-bit address

    SRecord::Buffer buffer;

    int number_bytes_written_to_buffer = WritePrimitiveRecord(buffer, record, 2);

    device->write( buffer.data(), number_bytes_written_to_buffer );
}

static void WriteSRecord6(const SRecord &record, QIODevice *device)
{
    Q_ASSERT(device);
    Q_ASSERT( record.address <= 0x00FFFFFF); // 24-bit address

    SRecord::Buffer buffer;

    int number_bytes_written_to_buffer = WritePrimitiveRecord(buffer, record, 3);

    device->write( buffer.data(), number_bytes_written_to_buffer );
}

static void WriteSRecord7(const SRecord &record, QIODevice *device)
{
    Q_ASSERT(device);
    Q_ASSERT( record.address <= 0xFFFFFFFF); // 32-bit address

    SRecord::Buffer buffer;

    int number_bytes_written_to_buffer = WritePrimitiveRecord(buffer, record, 4);

    device->write( buffer.data(), number_bytes_written_to_buffer );
}

static void WriteSRecord8(const SRecord &record, QIODevice *device)
{
    Q_ASSERT(device);
    Q_ASSERT( record.address <= 0x00FFFFFF); // 24-bit address

    SRecord::Buffer buffer;

    int number_bytes_written_to_buffer = WritePrimitiveRecord(buffer, record, 3);

    device->write( buffer.data(), number_bytes_written_to_buffer );
}

static void WriteSRecord9(const SRecord &record, QIODevice *device)
{
    Q_ASSERT(device);
    Q_ASSERT( record.address <= 0x0000FFFF); // 16-bit address

    SRecord::Buffer buffer;

    int number_bytes_written_to_buffer = WritePrimitiveRecord(buffer, record, 2);

    device->write( buffer.data(), number_bytes_written_to_buffer );
}

QSRecordStream::QSRecordStream(QIODevice *device)
    :
    _device(device)
{
}

QSRecordStream::~QSRecordStream()
{
    _device = nullptr;
}

void QSRecordStream::write(const SRecord &record)
{
    Q_ASSERT( (record.type >= 0) && (record.type <= 9) );
    Q_ASSERT( record.data.size() <= 0xFF );
    Q_ASSERT( device() );

    switch (record.type)
    {
    case 0:
        WriteSRecord0(record, device());
        break;
    case 1:
        WriteSRecord1(record, device());
        break;
    case 2:
        WriteSRecord2(record, device());
        break;
    case 3:
        WriteSRecord3(record, device());
        break;
    case 4:
        // Not defined!
        break;
    case 5:
        WriteSRecord5(record, device());
        break;
    case 6:
        WriteSRecord6(record, device());
        break;
    case 7:
        WriteSRecord7(record, device());
        break;
    case 8:
        WriteSRecord8(record, device());
        break;
    case 9:
        WriteSRecord9(record, device());
        break;
    default:
        break;
    }
}

OptionalSRecord QSRecordStream::read()
{
    SRecord::Buffer buffer;
    int line_length = device()->readLine(buffer.data(), buffer.size());

    if ( line_length == -1 )
        return std::nullopt;

    OptionalSRecord record = ReadSRecord( TrimWhitespace( std::string_view(buffer.data(), line_length) ) );

    if ( !record.has_value() )
        return std::nullopt;

    return record;
}

OptionalSRecords QSRecordStream::readAll()
{
    SRecords records;

    while (true && !device()->atEnd())
    {
        OptionalSRecord single_record = read();

        if ( single_record.has_value() )
            records.push_back( single_record.value() );
    }
    if (records.empty())
        return std::nullopt;

    return { std::move(records) };
}

OptionalSRecord QSRecordStream::ReadSRecord(std::string_view data)
{
    const int CharsPerByte = 2;

    if ( data.empty() )
        return { };

    if ( !ReadSRecordStart(data) )
        return { };

    int type = -1, byte_count = 0, line_length = static_cast<int>(data.size());

    if ( (type = ReadSRecordType(data)) == -1 )
        return { };

    if ( (byte_count = ReadSRecordByteCount(data)) == -1 )
        return { };

    if ( !SRecordByteCountIsValid(byte_count) )
        return { };

    if ( line_length < (byte_count * CharsPerByte) ) // Two hex chars per "byte", remember
        return { };

    OptionalSRecord record = std::nullopt;

    switch (type)
    {
    case 0:
        record = ReadSRecordType0(data);
        break;
    case 1:
        record = ReadSRecordType1(data);
        break;
    case 2:
        record = ReadSRecordType2(data);
        break;
    case 3:
        record = ReadSRecordType3(data);
        break;
    case 4:
        record = ReadSRecordType4(data);
        break;
    case 5:
        record = ReadSRecordType5(data);
        break;
    case 6:
        record = ReadSRecordType6(data);
        break;
    case 7:
        record = ReadSRecordType7(data);
        break;
    case 8:
        record = ReadSRecordType8(data);
        break;
    case 9:
        record = ReadSRecordType9(data);
        break;
    default:
        break;
    }
    return record;
}
