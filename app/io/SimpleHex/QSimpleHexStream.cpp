#include "io/SimpleHex/QSimpleHexStream.hpp"
#include <QIODevice>
#include <cctype>
#include <string_view>


static constexpr inline bool IsDecimalDigit(int digit)
{
    return (digit >= '0') && (digit <= '9');
}

static constexpr inline bool IsHexDigit(int digit)
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
            break;

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

static inline bool BeginsWith8BitHexValue(std::string_view data)
{
    return ( data.size() >= 2 ) && std::isxdigit( data[0] ) && std::isxdigit( data[1] );
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

    while ( !data.empty() )
    {
        data = EatWhiteSpace( data );
        if ( !BeginsWith8BitHexValue( data ) )
            break;

        int byte_value;

        if ( data.size() < 2 )
            byte_value = Read8BitHexValue( data );
        else
            byte_value = Read8BitHexValue( data.substr(0, 2) );

        if ( byte_value == -1 )
            break;

        bytes.push_back( static_cast<uint8_t>(byte_value) );

        // Strip off the hex representation we just read...
        data.remove_prefix( 2 ); // 8-bit value, thus 2 characters
    }
    return { std::make_pair( value, bytes ) };
}

QSimpleHexStream::QSimpleHexStream(QIODevice *device)
    :
    _device(device)
{
    Q_ASSERT( _device );
}

QSimpleHexStream::~QSimpleHexStream()
{
    _device = nullptr;
}

void QSimpleHexStream::write(const MemoryBlock &block)
{
    Q_ASSERT( device() );
    Q_ASSERT( device()->isOpen() );
}

OptionalMemoryBlock QSimpleHexStream::read()
{
    Q_ASSERT( device() );
    Q_ASSERT( device()->isOpen() );

    QByteArray line = device()->readLine();

    auto input_data = ReadSimpleHexLine( std::string_view( line.data(), line.size() ) );

    if ( !input_data.has_value() )
        return std::nullopt;

    return input_data.value();
}

OptionalMemoryBlocks QSimpleHexStream::readAll()
{
    Q_ASSERT( device() );
    Q_ASSERT( device()->isOpen() );

    // We expect lines of the form: XXXX:( XX)*
    // A 16-bit address followed by a colon folowed by one or more 2-digit
    // hex chars (a byte value).
    MemoryBlocks data;

    while (true)
    {
        auto line = read();

        if ( !line.has_value() )
            break;

        data.emplace_back( line.value() );
    }
    return data;
}
