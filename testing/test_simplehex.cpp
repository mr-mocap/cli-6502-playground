#include "test_simplehex.hpp"
#include "io/SimpleHex/QSimpleHexStream.hpp"
#include <QIODevice>
#include <QByteArray>
#include <QBuffer>
#include <iostream>
#include <cassert>

namespace SimpleHexTests
{

void ReadTypicalSingleLine()
{
    std::cout << "ReadTypicalSingleLine...";

    QByteArray input_data;

    input_data.append("0000: 12 34 56 78");

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSimpleHexStream stream( &input_buffer );
    OptionalMemoryBlock line = stream.read();

    assert( line.has_value() );
    assert( line.value().first == 0 );
    assert( line.value().second.size() == 4 );
    assert( line.value().second[0] == 0x12 );
    assert( line.value().second[1] == 0x34 );
    assert( line.value().second[2] == 0x56 );
    assert( line.value().second[3] == 0x78 );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadTypicalSingleLineHasEOL()
{
    std::cout << "ReadTypicalSingleLineHasEOL...";

    QByteArray input_data;

    input_data.append("0000: 12 34 56 78\n");

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSimpleHexStream stream( &input_buffer );
    OptionalMemoryBlock line = stream.read();

    assert( line.has_value() );
    assert( line.value().first == 0 );
    assert( line.value().second.size() == 4 );
    assert( line.value().second[0] == 0x12 );
    assert( line.value().second[1] == 0x34 );
    assert( line.value().second[2] == 0x56 );
    assert( line.value().second[3] == 0x78 );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadTypicalSingleLineWithNoSpacesIsNotAnError()
{
    std::cout << "ReadTypicalSingleLineWithNoSpacesIsNotAnError...";

    QByteArray input_data;

    input_data.append("0000:12345678");

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSimpleHexStream stream( &input_buffer );
    OptionalMemoryBlock line = stream.read();

    assert( line.has_value() );
    assert( line.value().first == 0 );
    assert( line.value().second.size() == 4 );
    assert( line.value().second[0] == 0x12 );
    assert( line.value().second[1] == 0x34 );
    assert( line.value().second[2] == 0x56 );
    assert( line.value().second[3] == 0x78 );

    std::cout << "SUCCESS!" << std::endl;
}

void HexDigitsMustBeNextToEachOtherInPairs()
{
    std::cout << "HexDigitsMustBeNextToEachOtherInPairs...";

    {
        QByteArray input_data;

        input_data.append("0000:1 2");

        QBuffer input_buffer( &input_data );

        input_buffer.open( QIODevice::ReadOnly );

        assert( input_buffer.isOpen() );

        QSimpleHexStream stream( &input_buffer );
        OptionalMemoryBlock line = stream.read();

        assert( line.has_value() );
        assert( line.value().first == 0 );
        assert( line.value().second.empty() );
    }
    {
        QByteArray input_data;

        input_data.append("0000: 12 3");

        QBuffer input_buffer( &input_data );

        input_buffer.open( QIODevice::ReadOnly );

        assert( input_buffer.isOpen() );

        QSimpleHexStream stream( &input_buffer );
        OptionalMemoryBlock line = stream.read();

        assert( line.has_value() );
        assert( line.value().first == 0 );
        assert( line.value().second.size() == 1 );
        assert( line.value().second[0] == 0x12 );
    }
    {
        QByteArray input_data;

        input_data.append("0000: 1 2 3");

        QBuffer input_buffer( &input_data );

        input_buffer.open( QIODevice::ReadOnly );

        assert( input_buffer.isOpen() );

        QSimpleHexStream stream( &input_buffer );
        OptionalMemoryBlock line = stream.read();

        assert( line.has_value() );
        assert( line.value().first == 0 );
        assert( line.value().second.empty() );
    }

    std::cout << "SUCCESS!" << std::endl;
}

void BeginningOfLineMustBeFourHexDigitsAndAColon()
{
    std::cout << "BeginningOfLineMustBeFourHexDigitsAndAColon...";

    {
        QByteArray input_data;

        input_data.append("12");

        QBuffer input_buffer( &input_data );

        input_buffer.open( QIODevice::ReadOnly );

        assert( input_buffer.isOpen() );

        QSimpleHexStream stream( &input_buffer );
        OptionalMemoryBlock line = stream.read();

        assert( !line.has_value() );
    }
    {
        QByteArray input_data;

        input_data.append("1234");

        QBuffer input_buffer( &input_data );

        input_buffer.open( QIODevice::ReadOnly );

        assert( input_buffer.isOpen() );

        QSimpleHexStream stream( &input_buffer );
        OptionalMemoryBlock line = stream.read();

        assert( !line.has_value() );
    }
    {
        QByteArray input_data;

        input_data.append("1234:");

        QBuffer input_buffer( &input_data );

        input_buffer.open( QIODevice::ReadOnly );

        assert( input_buffer.isOpen() );

        QSimpleHexStream stream( &input_buffer );
        OptionalMemoryBlock line = stream.read();

        assert( line.has_value() );
        assert( line.value().first == 0x1234 );
        assert( line.value().second.empty() );
    }

    std::cout << "SUCCESS!" << std::endl;
}

void ReadSimpleHexTests()
{
    ReadTypicalSingleLine();
    ReadTypicalSingleLineHasEOL();
    ReadTypicalSingleLineWithNoSpacesIsNotAnError();
    HexDigitsMustBeNextToEachOtherInPairs();
    BeginningOfLineMustBeFourHexDigitsAndAColon();
}

void WriteSimpleHexTests()
{
}

void Run()
{
    std::cout << "Running SimpleHexTests" << std::endl;

    ReadSimpleHexTests();
    WriteSimpleHexTests();
}

}
