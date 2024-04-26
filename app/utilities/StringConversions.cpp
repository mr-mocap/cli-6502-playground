#include "utilities/StringConversions.hpp"

#include <string>

using namespace std::literals;

bool IsDecimalDigit(int digit)
{
    return "0123456789"sv.find( std::char_traits<char>::to_char_type( digit ) ) != std::string_view::npos;
}

bool IsHexDigit(int digit)
{
    return "0123456789abcdefABCDEF"sv.find( std::char_traits<char>::to_char_type( digit ) ) != std::string_view::npos;
}

int HexDigitsBigEndianToDecimal(std::string_view hex_digits_in_ascii)
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

int Read8BitHexValue(std::string_view data)
{
    return HexDigitsBigEndianToDecimal( data );
}

int Read16BitHexValue(std::string_view data)
{
    return HexDigitsBigEndianToDecimal( data );
}

int Read24BitHexValue(std::string_view data)
{
    return HexDigitsBigEndianToDecimal( data );
}

int Read32BitHexValue(std::string_view data)
{
    return HexDigitsBigEndianToDecimal( data );
}
