#include "test_srecord.hpp"
#include "io/SRecord/QSRecordStream.hpp"
#include <QIODevice>
#include <QByteArray>
#include <QBuffer>
#include <iostream>
#include <cassert>

using namespace std::literals::string_view_literals;

namespace SRecordTests
{

static const char *ValidExampleS0Record = "S00600004844521B";
static const char *ValidExampleS1Record = "S106C0004844525B";
static const char *ValidExampleS2Record = "S207AABBCC484452E9";
static const char *ValidExampleS3Record = "S308AABBCCDD4844520B";
static const char *ValidExampleS4Record = "S408AABBCCDD484452E9";
static const char *ValidExampleS5Record = "S5031234B6";
static const char *ValidExampleS6Record = "S6041234565F";
static const char *ValidExampleS7Record = "S70512345678E6";
static const char *ValidExampleS8Record = "S8041234565F";
static const char *ValidExampleS9Record = "S9031234B6";

void ReadSRecordInterpretsS0()
{
    std::cout << "ReadSRecordInterpretsS0...";

    QByteArray input_data;

    input_data.append(ValidExampleS0Record);

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSRecordStream stream( &input_buffer );
    OptionalSRecord record = stream.read();

    assert( record.has_value() );
    assert( record->type == 0 );
    assert( record->address == 0 );
    assert( record->checksum == 0x1B );
    assert( record->data.size() == 3 );
    assert( record->data[0] == 'H' );
    assert( record->data[1] == 'D' );
    assert( record->data[2] == 'R' );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadSRecordInterpretsS1()
{
    std::cout << "ReadSRecordInterpretsS1...";

    QByteArray input_data;

    input_data.append(ValidExampleS1Record);

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSRecordStream stream( &input_buffer );
    OptionalSRecord record = stream.read();

    assert( record.has_value() );
    assert( record->type == 1 );
    assert( record->address == 0xC000 );
    assert( record->checksum == 0x5B );
    assert( record->data.size() == 3 );
    assert( record->data[0] == 'H' );
    assert( record->data[1] == 'D' );
    assert( record->data[2] == 'R' );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadSRecordInterpretsS2()
{
    std::cout << "ReadSRecordInterpretsS2...";

    QByteArray input_data;

    input_data.append(ValidExampleS2Record);

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSRecordStream stream( &input_buffer );
    OptionalSRecord record = stream.read();

    assert( record.has_value() );
    assert( record->type == 2 );
    assert( record->address == 0x00AABBCC );
    assert( record->checksum == 0xE9 );
    assert( record->data.size() == 3 );
    assert( record->data[0] == 'H' );
    assert( record->data[1] == 'D' );
    assert( record->data[2] == 'R' );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadSRecordInterpretsS3()
{
    std::cout << "ReadSRecordInterpretsS3...";

    QByteArray input_data;

    input_data.append(ValidExampleS3Record);

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSRecordStream stream( &input_buffer );
    OptionalSRecord record = stream.read();

    assert( record.has_value() );
    assert( record->type == 3 );
    assert( record->address == 0xAABBCCDD );
    assert( record->checksum == 0x0B );
    assert( record->data.size() == 3 );
    assert( record->data[0] == 'H' );
    assert( record->data[1] == 'D' );
    assert( record->data[2] == 'R' );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadSRecordInterpretsS4()
{
    std::cout << "ReadSRecordInterpretsS4...";

    QByteArray input_data;

    input_data.append(ValidExampleS4Record);

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSRecordStream stream( &input_buffer );
    OptionalSRecord record = stream.read();

    assert( !record.has_value() );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadSRecordInterpretsS5()
{
    std::cout << "ReadSRecordInterpretsS5...";

    QByteArray input_data;

    input_data.append(ValidExampleS5Record);

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSRecordStream stream( &input_buffer );
    OptionalSRecord record = stream.read();

    assert( record.has_value() );
    assert( record->type == 5 );
    assert( record->address == 0x1234 );
    assert( record->checksum == 0xB6 );
    assert( record->data.empty() );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadSRecordInterpretsS6()
{
    std::cout << "ReadSRecordInterpretsS6...";

    QByteArray input_data;

    input_data.append(ValidExampleS6Record);

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSRecordStream stream( &input_buffer );
    OptionalSRecord record = stream.read();

    assert( record.has_value() );
    assert( record->type == 6 );
    assert( record->address == 0x123456 );
    assert( record->checksum == 0x5F );
    assert( record->data.empty() );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadSRecordInterpretsS7()
{
    std::cout << "ReadSRecordInterpretsS7...";

    QByteArray input_data;

    input_data.append(ValidExampleS7Record);

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSRecordStream stream( &input_buffer );
    OptionalSRecord record = stream.read();

    assert( record.has_value() );
    assert( record->type == 7 );
    assert( record->address == 0x12345678 );
    assert( record->checksum == 0xE6 );
    assert( record->data.empty() );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadSRecordInterpretsS8()
{
    std::cout << "ReadSRecordInterpretsS8...";

    QByteArray input_data;

    input_data.append(ValidExampleS8Record);

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSRecordStream stream( &input_buffer );
    OptionalSRecord record = stream.read();

    assert( record.has_value() );
    assert( record->type == 8 );
    assert( record->address == 0x123456 );
    assert( record->checksum == 0x5F );
    assert( record->data.empty() );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadSRecordInterpretsS9()
{
    std::cout << "ReadSRecordInterpretsS9...";

    QByteArray input_data;

    input_data.append(ValidExampleS9Record);

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSRecordStream stream( &input_buffer );

    OptionalSRecord record = stream.read();

    assert( record.has_value() );
    assert( record->type == 9 );
    assert( record->address == 0x1234 );
    assert( record->checksum == 0xB6 );
    assert( record->data.empty() );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadSRecordValidSequence0Through9()
{
    std::cout << "ReadSRecordValidSequence0Through9...";

    QByteArray input_data;

    input_data.append(ValidExampleS0Record).append('\n');
    input_data.append(ValidExampleS1Record).append('\n');
    input_data.append(ValidExampleS2Record).append('\n');
    input_data.append(ValidExampleS3Record).append('\n');
    input_data.append(ValidExampleS5Record).append('\n');
    input_data.append(ValidExampleS6Record).append('\n');
    input_data.append(ValidExampleS7Record).append('\n');
    input_data.append(ValidExampleS8Record).append('\n');
    input_data.append(ValidExampleS9Record);

    QBuffer input_buffer( &input_data );

    input_buffer.open( QIODevice::ReadOnly );

    assert( input_buffer.isOpen() );

    QSRecordStream stream( &input_buffer );
    OptionalSRecords records_read = stream.readAll();

    assert( records_read.has_value() );
    assert( records_read.value().size() == 9 );

    // Check the records in order

    // S0
    assert( records_read.value()[0].type == 0 );
    assert( records_read.value()[0].address == 0 );
    assert( records_read.value()[0].checksum == 0x1B );
    assert( records_read.value()[0].data.size() == 3 );
    assert( records_read.value()[0].data[0] == 'H' );
    assert( records_read.value()[0].data[1] == 'D' );
    assert( records_read.value()[0].data[2] == 'R' );

    // S1
    assert( records_read.value()[1].type == 1 );
    assert( records_read.value()[1].address == 0xC000 );
    assert( records_read.value()[1].checksum == 0x5B );
    assert( records_read.value()[1].data.size() == 3 );
    assert( records_read.value()[1].data[0] == 'H' );
    assert( records_read.value()[1].data[1] == 'D' );
    assert( records_read.value()[1].data[2] == 'R' );

    // S2
    assert( records_read.value()[2].type == 2 );
    assert( records_read.value()[2].address == 0x00AABBCC );
    assert( records_read.value()[2].checksum == 0xE9 );
    assert( records_read.value()[2].data.size() == 3 );
    assert( records_read.value()[2].data[0] == 'H' );
    assert( records_read.value()[2].data[1] == 'D' );
    assert( records_read.value()[2].data[2] == 'R' );

    // S3
    assert( records_read.value()[3].type == 3 );
    assert( records_read.value()[3].address == 0xAABBCCDD );
    assert( records_read.value()[3].checksum == 0x0B );
    assert( records_read.value()[3].data.size() == 3 );
    assert( records_read.value()[3].data[0] == 'H' );
    assert( records_read.value()[3].data[1] == 'D' );
    assert( records_read.value()[3].data[2] == 'R' );

    // S4 (NONE)

    // S5
    assert( records_read.value()[4].type == 5 );
    assert( records_read.value()[4].address == 0x1234 );
    assert( records_read.value()[4].checksum == 0xB6 );
    assert( records_read.value()[4].data.empty() );

    // S6
    assert( records_read.value()[5].type == 6 );
    assert( records_read.value()[5].address == 0x123456 );
    assert( records_read.value()[5].checksum == 0x5F );
    assert( records_read.value()[5].data.empty() );

    // S7
    assert( records_read.value()[6].type == 7 );
    assert( records_read.value()[6].address == 0x12345678 );
    assert( records_read.value()[6].checksum == 0xE6 );
    assert( records_read.value()[6].data.empty() );

    // S8
    assert( records_read.value()[7].type == 8 );
    assert( records_read.value()[7].address == 0x123456 );
    assert( records_read.value()[7].checksum == 0x5F );
    assert( records_read.value()[7].data.empty() );

    // S9
    assert( records_read.value()[8].type == 9 );
    assert( records_read.value()[8].address == 0x1234 );
    assert( records_read.value()[8].checksum == 0xB6 );
    assert( records_read.value()[8].data.empty() );

    std::cout << "SUCCESS!" << std::endl;
}

void ReadSRecordTests()
{
    // Basically do the same tests as the individual record types
    ReadSRecordInterpretsS0();
    ReadSRecordInterpretsS1();
    ReadSRecordInterpretsS2();
    ReadSRecordInterpretsS3();
    ReadSRecordInterpretsS4();
    ReadSRecordInterpretsS5();
    ReadSRecordInterpretsS6();
    ReadSRecordInterpretsS7();
    ReadSRecordInterpretsS8();
    ReadSRecordInterpretsS9();

    ReadSRecordValidSequence0Through9();
}

void WriteSRecordInterpretsS0()
{
    std::cout << "WriteSRecordInterpretsS0...";

    QBuffer output;
    SRecord record(0, 0, { 'H', 'D', 'R' });

    output.open( QIODevice::WriteOnly );

    QSRecordStream stream( &output );

    stream.write( record );

    QByteArray &buffer = output.buffer();

    assert( buffer.compare(ValidExampleS0Record) == 0 );

    std::cout << "SUCCESS!" << std::endl;
}

void WriteSRecordInterpretsS1()
{
    std::cout << "WriteSRecordInterpretsS1...";

    QBuffer output;
    SRecord record(1, 0xC000, { 'H', 'D', 'R' });

    output.open( QIODevice::WriteOnly );

    QSRecordStream stream( &output );

    stream.write( record );

    QByteArray &buffer = output.buffer();

    assert( buffer.compare(ValidExampleS1Record) == 0 );

    std::cout << "SUCCESS!" << std::endl;
}

void WriteSRecordInterpretsS2()
{
    std::cout << "WriteSRecordInterpretsS2...";

    QBuffer output;
    SRecord record(2, 0xAABBCC, { 'H', 'D', 'R' });

    output.open( QIODevice::WriteOnly );

    QSRecordStream stream( &output );

    stream.write( record );

    QByteArray &buffer = output.buffer();

    assert( buffer.compare(ValidExampleS2Record) == 0 );

    std::cout << "SUCCESS!" << std::endl;
}

void WriteSRecordInterpretsS3()
{
    std::cout << "WriteSRecordInterpretsS3...";

    QBuffer output;
    SRecord record(3, 0xAABBCCDD, { 'H', 'D', 'R' });

    output.open( QIODevice::WriteOnly );

    QSRecordStream stream( &output );

    stream.write( record );

    QByteArray &buffer = output.buffer();

    assert( buffer.compare(ValidExampleS3Record) == 0 );

    std::cout << "SUCCESS!" << std::endl;
}

void WriteSRecordInterpretsS5()
{
    std::cout << "WriteSRecordInterpretsS5...";

    QBuffer output;
    SRecord record(5, 0x1234, { });

    output.open( QIODevice::WriteOnly );

    QSRecordStream stream( &output );

    stream.write( record );

    QByteArray &buffer = output.buffer();

    assert( buffer.compare(ValidExampleS5Record) == 0 );

    std::cout << "SUCCESS!" << std::endl;
}

void WriteSRecordInterpretsS6()
{
    std::cout << "WriteSRecordInterpretsS6...";

    QBuffer output;
    SRecord record(6, 0x123456, { });

    output.open( QIODevice::WriteOnly );

    QSRecordStream stream( &output );

    stream.write( record );

    QByteArray &buffer = output.buffer();

    assert( buffer.compare(ValidExampleS6Record) == 0 );

    std::cout << "SUCCESS!" << std::endl;
}

void WriteSRecordInterpretsS7()
{
    std::cout << "WriteSRecordInterpretsS7...";

    QBuffer output;
    SRecord record(7, 0x12345678, { });

    output.open( QIODevice::WriteOnly );

    QSRecordStream stream( &output );

    stream.write( record );

    QByteArray &buffer = output.buffer();

    assert( buffer.compare(ValidExampleS7Record) == 0 );

    std::cout << "SUCCESS!" << std::endl;
}

void WriteSRecordInterpretsS8()
{
    std::cout << "WriteSRecordInterpretsS8...";

    QBuffer output;
    SRecord record(8, 0x123456, { });

    output.open( QIODevice::WriteOnly );

    QSRecordStream stream( &output );

    stream.write( record );

    QByteArray &buffer = output.buffer();

    assert( buffer.compare(ValidExampleS8Record) == 0 );

    std::cout << "SUCCESS!" << std::endl;
}

void WriteSRecordInterpretsS9()
{
    std::cout << "WriteSRecordInterpretsS9...";

    QBuffer output;
    SRecord record(9, 0x1234, { });

    output.open( QIODevice::WriteOnly );

    QSRecordStream stream( &output );

    stream.write( record );

    QByteArray &buffer = output.buffer();

    assert( buffer.compare(ValidExampleS9Record) == 0 );

    std::cout << "SUCCESS!" << std::endl;
}

void WriteSRecordTests()
{
    WriteSRecordInterpretsS0();
    WriteSRecordInterpretsS1();
    WriteSRecordInterpretsS2();
    WriteSRecordInterpretsS3();

    WriteSRecordInterpretsS5();
    WriteSRecordInterpretsS6();
    WriteSRecordInterpretsS7();
    WriteSRecordInterpretsS8();
    WriteSRecordInterpretsS9();
}

void Run()
{
    std::cout << "Running SRecordTests" << std::endl;

    ReadSRecordTests();
    WriteSRecordTests();
}

}
