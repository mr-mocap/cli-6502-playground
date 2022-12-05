#ifndef IO_HPP
#define IO_HPP

#include <QIODevice>
#include <optional>
#include <vector>
#include <map>
#include <string_view>

class IOInterface
{
public:
    virtual void ReadInputFrom(QIODevice *device) = 0;
};


class IOSimpleHex : public IOInterface
{
public:
    using Bytes = std::vector<uint8_t>;

    // We expect lines of the form: XXXX:( XX)*
    // A 16-bit hex addres followed by a colon folowed by one or more 2-digit
    // hex chars (a byte value).
    void ReadInputFrom(QIODevice *device) override;

protected:
    std::map<int, Bytes> _data;
};

class IOSRecord : public IOInterface
{
public:
    using Bytes = std::vector<char>;

    struct Record
    {
        Record(int a, const Bytes &b, int c) : address(a), data(b), checksum(c) { }

        int   address = 0;
        Bytes data;
        int   checksum = 0;
    };

    using Records = std::vector<Record>;
    using RecordData = std::optional<Record>;

    void ReadInputFrom(QIODevice *device) override;

    std::optional<Records> records;
protected:
    bool readRecordStart(const std::string_view data);
    int  readRecordType(const std::string_view data);
    int  readRecordByteCount(std::string_view data);

    RecordData readRecordType0(std::string_view data);
    RecordData readRecordType1(std::string_view data);
    RecordData readRecordType2(std::string_view data);
    RecordData readRecordType3(std::string_view data);
    // Type 4 is reserved
    RecordData readRecordType5(std::string_view data);
    RecordData readRecordType6(std::string_view data);
    RecordData readRecordType7(std::string_view data);
    RecordData readRecordType8(std::string_view data);
    RecordData readRecordType9(std::string_view data);
};

#endif // IO_HPP
