#ifndef STRINGCONVERSIONS_HPP
#define STRINGCONVERSIONS_HPP

#include <string_view>
#include <cctype>


bool IsDecimalDigit(int digit);

bool IsHexDigit(int digit);

int HexDigitsBigEndianToDecimal(std::string_view hex_digits_in_ascii);

int Read8BitHexValue(std::string_view data);

int Read16BitHexValue(std::string_view data);

int Read24BitHexValue(std::string_view data);

int Read32BitHexValue(std::string_view data);

inline bool BeginsWith8BitHexValue(std::string_view data)
{
    return ( data.size() >= 2 ) && IsHexDigit( data[0] ) && IsHexDigit( data[1] );
}

constexpr inline std::string_view EatLeadingWhiteSpace(std::string_view data)
{
    while ( !data.empty() && std::isspace(data.front()) )
        data.remove_prefix( 1 );
    return data;
}

constexpr inline std::string_view EatTrailingWhiteSpace(std::string_view data)
{
    while ( !data.empty() && std::isspace(data.back()) )
        data.remove_suffix( 1 );
    return data;
}

constexpr inline std::string_view TrimWhitespace(std::string_view str)
{
    return EatTrailingWhiteSpace( EatLeadingWhiteSpace( str ) );
}

#endif // STRINGCONVERSIONS_HPP
