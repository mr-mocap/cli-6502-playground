#ifndef IBUSDEVICE_HPP
#define IBUSDEVICE_HPP

#include <QObject>

class IBusDevice : public QObject
{
    Q_OBJECT
public:
    using addressType = uint16_t;

    explicit IBusDevice(addressType lower_address,
                        addressType upper_address,
                        bool     writable,
                        bool     readable,
                        QObject *parent = nullptr);
    virtual ~IBusDevice() = 0;

    bool writable() const { return _writable; }
    bool readable() const { return _readable; }

    addressType lowerAddress() const { return _lower_address_range; }
    addressType upperAddress() const { return _upper_address_range; }

    bool handlesAddress(addressType address) const;

signals:

public slots:
    void    write(addressType address, uint8_t data);
    uint8_t read(addressType address, bool read_only);

protected:
    virtual void    writeImplementation(addressType address, uint8_t data) = 0;
    virtual uint8_t readImplementation(addressType address, bool read_only) = 0;

private:
    addressType _lower_address_range = 0;
    addressType _upper_address_range = 0;
    bool        _writable = false;
    bool        _readable = false;
};

#endif // IBUSDEVICE_HPP
