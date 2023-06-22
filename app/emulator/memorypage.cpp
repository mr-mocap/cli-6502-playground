#include "memorypage.hpp"
#include "ftxui/component/component.hpp"
#include "pageview.hpp"
#include <utility>

using namespace ftxui;

inline int LowNybble(int value)
{
    return value & 0x0F;
}

inline int HighNybble(int value)
{
    return value & 0xF0;
}

inline int LowNybbleToHighNybble(int value)
{
    return HighNybble( value << 4 );
}

inline int HighNybbleToLowNybble(int value)
{
    return LowNybble( value >> 4 );
}

inline int IncrementLowNybble(int value)
{
    return HighNybble(value) | LowNybble(value + 0x01);
}

inline int IncrementHighNybble(int value)
{
    return HighNybble(value + 0x10) | LowNybble(value);
}

inline int DecrementLowNybble(int value)
{
    return HighNybble(value) | LowNybble(value - 0x01);
}

inline int DecrementHighNybble(int value)
{
    return HighNybble(value - 0x10) | LowNybble(value);
}

class MemoryPageComponent : public ComponentBase
{
public:
    MemoryPageComponent(Ref<MemoryPageOption> option);

    Element Render() override;

    bool OnEvent(Event event) override;

    bool Focusable() const final { return true; }

    void SetActiveChild(ComponentBase* /*child*/) override;

    int currentByte() { return _option->current_byte(); }

    void setCurrentByte(int new_position)
    {
        if (new_position != currentByte())
            _option->current_byte() = new_position;
    }

    bool editMode() const { return _edit_mode; }

protected:
    Ref<MemoryPageOption> _option;
    Box                   _box;
    bool                  _hovered = false;
    bool                  _edit_mode = false; // We can change the memory value while in edit mode
    bool                  _was_focused = false;

    bool OnMouseEvent(Event &event);
    bool OnEditModeEvent(Event &event);
    void OnFocusChanged(bool new_focus_state);
};

MemoryPageComponent::MemoryPageComponent(Ref<MemoryPageOption> option)
    :
    _option{ std::move(option) }
{
}

Element MemoryPageComponent::Render()
{
    if ( Focused() != _was_focused )
        OnFocusChanged( Focused() );

    if ( Focused() )
        return pageview( _option->model,
                         Ref<int>(&_option->current_byte()),
                         Ref<int>(&_option->show_pc()),
                         _option->edit_mode_decorator ) | reflect(_box);
    else
        return pageview( _option->model, Ref<int>(-1), Ref<int>(&_option->show_pc()), _option->edit_mode_decorator ) | reflect(_box);
}

void MemoryPageComponent::OnFocusChanged(bool new_focus_state)
{
    if ( new_focus_state )
    {
        if ( currentByte() == -1 )
        {
            // Check the previous event to see if we came in from a particular direction
            if ( _option->previous_event() == Event::ArrowDown )
                setCurrentByte( 0x07 );
            else if ( _option->previous_event() == Event::ArrowUp )
                setCurrentByte( 0xF7 );
            else if ( _option->previous_event() == Event::ArrowLeft )
                setCurrentByte( 0x7F );
            else if ( _option->previous_event() == Event::ArrowRight )
                setCurrentByte( 0x70 );
        }
    }
}

bool MemoryPageComponent::OnEvent(Event event)
{
    if ( event.is_mouse() )
        return OnMouseEvent(event);

    if ( Focused() )
    {
        if (_edit_mode)
            return OnEditModeEvent(event);

        if (event == Event::ArrowDown)
        {
            // Increment the high nybble of the byte
            if ( HighNybbleToLowNybble( currentByte() ) < 0x0F)
            {
                // Prevent wrap-around
                setCurrentByte( IncrementHighNybble( currentByte() ) );
                return true;
            }
        }
        else if (event == Event::ArrowUp)
        {
            // Decrement the high nybble of the byte
            if ( HighNybbleToLowNybble( currentByte() ) > 0x00)
            {
                // Prevent wrap-around
                setCurrentByte( DecrementHighNybble( currentByte() ) );
                return true;
            }
        }
        else if (event == Event::ArrowLeft)
        {
            // Decrement the low nybble of the byte
            if ( LowNybble( currentByte() ) > 0x00 )
            {
                // Prevent wrap-around
                setCurrentByte( DecrementLowNybble( currentByte() ) );
                return true;
            }
        }
        else if (event == Event::ArrowRight)
        {
            // Increment the low nybble of the byte
            if ( LowNybble( currentByte() ) < 0x0F)
            {
                // Prevent wrap-around
                setCurrentByte( IncrementLowNybble( currentByte() ) );
                return true;
            }
        }
        else if (event == Event::Home)
        {
            setCurrentByte( 0 );
            return true;
        }
        else if (event == Event::End)
        {
            setCurrentByte( 0xFF );
            return true;
        }
        else if (event == Event::PageUp)
        {
            _option->model->setPage( (_option->model->page() - 1) & 0xFF );
            return true;
        }
        else if (event == Event::PageDown)
        {
            _option->model->setPage( (_option->model->page() + 1) & 0xFF );
            return true;
        }
        else if (event == Event::Return)
        {
            _edit_mode = true;
            return true;
        }
    }

    return false;
}

bool MemoryPageComponent::OnMouseEvent(Event &event)
{
    _hovered = _box.Contain(event.mouse().x, event.mouse().y) && CaptureMouse(event);
    if (_hovered) {
        int current_byte = pageview_byte(event.mouse().x - _box.x_min - 1, event.mouse().y - _box.y_min - 1);

        if (current_byte == -1)
            return false;
        else
        {
            setCurrentByte(current_byte);
            return true;
        }
    }
    else
    {
        setCurrentByte(-1);
        return false;
    }

    if (event.mouse().button != Mouse::Left ||
        event.mouse().motion != Mouse::Pressed) {
      return false;
    }

    TakeFocus();
    return true;
}

bool MemoryPageComponent::OnEditModeEvent(Event &event)
{
    if ( (event == Event::ArrowLeft) || (event == Event::ArrowDown) )
    {
        // Decrement memory location
        IBusDevice::addressType address = (_option->model->page() << 8) | currentByte();
        RamBusDevice::memory_type::value_type new_value = _option->model->model()->memory().at( address ) - 1;

        _option->model->model()->write( address, new_value );
        return true;
    }
    else if ( (event == Event::ArrowRight) || (event == Event::ArrowUp) )
    {
        // Increment memory location
        IBusDevice::addressType address = (_option->model->page() << 8) | currentByte();
        RamBusDevice::memory_type::value_type new_value = _option->model->model()->memory().at( address ) + 1;

        _option->model->model()->write( address, new_value );
        return true;
    }
    else if (event == Event::PageUp)
    {
        _option->model->setPage( (_option->model->page() - 1) & 0xFF );
        return true;
    }
    else if (event == Event::PageDown)
    {
        _option->model->setPage( (_option->model->page() + 1) & 0xFF );
        return true;
    }
    else if (event == Event::Return)
    {
        _edit_mode = false;
        return true;
    }

    return false;
}

void MemoryPageComponent::SetActiveChild(ComponentBase *child)
{
    if ( child == this )
        TakeFocus();
}

Component MemoryPage(Ref<MemoryPageOption> option)
{
    return Make<MemoryPageComponent>( std::move(option) );
}
