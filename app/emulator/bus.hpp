#ifndef BUS_HPP
#define BUS_HPP

#include <QObject>
#include <cstdint>


class Bus : public QObject
{
    Q_OBJECT
public:
    using addressType = uint16_t;

    explicit Bus(QObject *parent = nullptr);

    static constexpr addressType bitWidth()   { return 16; }
    static constexpr addressType minAddress() { return 0x00; }
    static constexpr addressType maxAddress() { return static_cast<addressType>(1 << (bitWidth() - 1)); }

public slots:
    void    write(addressType address, uint8_t data);
    uint8_t read(addressType address, bool read_only);

signals:
    void    busWritten(addressType address, uint8_t data);
    uint8_t busRead(addressType address, bool read_only);
};

#endif // BUS_HPP
