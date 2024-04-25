#ifndef QSIMPLEHEXSTREAM_HPP
#define QSIMPLEHEXSTREAM_HPP

#include "io/memory_block.hpp"
#include <optional>

using OptionalMemoryBlock = std::optional<MemoryBlock>;
using OptionalMemoryBlocks = std::optional<MemoryBlocks>;

class QIODevice;

class QSimpleHexStream
{
public:
    enum Status { Ok };

    explicit QSimpleHexStream(QIODevice *device);
    ~QSimpleHexStream();

    QIODevice *device() const { return _device; }

    void write(const MemoryBlock &block);

    OptionalMemoryBlock read();

    OptionalMemoryBlocks readAll();

protected:
    QIODevice *_device = nullptr;
};

#endif
