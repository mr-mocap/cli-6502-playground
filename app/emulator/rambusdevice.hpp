#ifndef RAMBUSDEVICE_HPP
#define RAMBUSDEVICE_HPP

#include "ibusdevice.hpp"
#include <array>


/** Represents a contiguous block of RAM.
 *
 */
class RamBusDevice : public IBusDevice
{
    Q_OBJECT
public:
    using memory_type = std::array<uint8_t, 64 * 1024>;

    RamBusDevice();
   ~RamBusDevice() override;

   /** Gives access to the memory.
    *
    *  This is for allowing another entity to view
    *  the current contents of the memory.
    *
    *  @return A reference to the underlying memory
    */
   const memory_type &memory() const { return _data; }

public slots:

signals:
    /** A signal representing what data was just written.
     *
     *  This is a pass-through of the @c write() method.
     *  It's main purpose is to allow for another entity
     *  to know when the underlying memory has changed.
     *
     *  @param address The address to write to
     *  @param data    The byte of data to write
     *
     *  @see write
     */
    void memoryChanged(addressType address, uint8_t data);

protected:
    void    writeImplementation(addressType address, uint8_t data) override;
    uint8_t readImplementation(addressType address, bool read_only) override;

private:
    memory_type _data;
};

#endif // RAMBUSDEVICE_HPP
