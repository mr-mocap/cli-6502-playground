#include "disassembly.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

inline int LowNybble(int value)
{
    return value & 0x0F;
}

inline int HighNybble(int value)
{
    return value & 0xF0;
}

inline int IncrementHighNybble(int value)
{
    return HighNybble(value + 0x10) | LowNybble(value);
}

inline int DecrementHighNybble(int value)
{
    return HighNybble(value - 0x10) | LowNybble(value);
}

inline int IncrementHighByte(int value)
{
    return ((value + 0x0100) & 0xFF00) | (value & 0x00FF);
}

inline int DecrementHighByte(int value)
{
    return ((value - 0x0100) & 0xFF00) | (value & 0x00FF);
}


class Disassembly : public ComponentBase
{
public:
    Disassembly( Ref<DisassemblyOption> options)
        :
        _options( std::move(options) )
    {
        _options().ram->connect( _options().ram, &RamBusDevice::memoryChanged,
                                [this](IBusDevice::addressType address, uint8_t data)
                                {
                                    _memoryChanged(address, data);
                                });
    }

    Element Render() override
    {
        // "$XXXX: NOP #$XXXX [$XXXX]"
        const int format_length = 25;
        int lines = _box.x_max - _box.x_min;
        Elements elements;

        if (lines > 0)
        {
            elements.reserve( lines );

            _options().start_address = _options().model->pc();
            _options().end_address   = _options().model->pc() + 127;

            if ( !_isInDisassembly(_options().start_address()) )
                _generateDisassembly();

            for (const auto &iCurrentInstruction : _disassembly)
            {
                if ( iCurrentInstruction.first == _options().model->pc() )
                    elements.emplace_back( text( iCurrentInstruction.second ) | bgcolor(Color::Blue) );
                else
                    elements.emplace_back( text( iCurrentInstruction.second ) );
            }
        }
        return vbox( elements ) | size(WIDTH, EQUAL, format_length) | size(HEIGHT, GREATER_THAN, 0) | reflect( _box );
    }

    bool Focusable() const final { return false; }


protected:
    Ref<DisassemblyOption> _options;
    Box                    _box;
    olc6502::disassemblyType _disassembly;

    void _memoryChanged(IBusDevice::addressType address, uint8_t data);
    bool _isInDisassembly(const olc6502::addressType address) const
    {
        return !_disassembly.empty() &&
               (_disassembly.cbegin()->first <= address) &&
               (_disassembly.crbegin()->first >= address);
    }

    void _generateDisassembly()
    {
        // Disassemble from the pc forward...
        _disassembly = _options().model->disassemble( _options().start_address(), _options().end_address() );
    }
};

void Disassembly::_memoryChanged(IBusDevice::addressType address, uint8_t data)
{
    Q_UNUSED(data)

    if ( _isInDisassembly(address) )
        _generateDisassembly();
}

ftxui::Component disassembly(ftxui::Ref<DisassemblyOption> options)
{
    return Make<Disassembly>( std::move(options) );
}
