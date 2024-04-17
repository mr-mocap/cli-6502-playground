#ifndef QSRECORDSTREAM_HPP
#define QSRECORDSTREAM_HPP

#include "srecord.hpp"

#include <string_view>

class QIODevice;

class QSRecordStream
{
public:
    enum Status { Ok };

    explicit QSRecordStream(QIODevice *device);
    virtual ~QSRecordStream();

    QIODevice *device() const { return _device; }

    void write(const SRecord &record);
    OptionalSRecord read();

    OptionalSRecords readAll();

protected:
    QIODevice *_device = nullptr;

    static OptionalSRecord ReadSRecord(std::string_view data);
};

#endif // QSRECORDSTREAM_HPP
