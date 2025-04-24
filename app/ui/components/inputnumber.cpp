#include "inputnumber.hpp"
#include <map>
#include <cmath>
#include <charconv>
#include <algorithm>

#include "ftxui/component/component.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

struct BaseProperties {
  Base                  base;
  int                   base_representation;
  int                   max_digits;
  std::string           base_prefix;
  int                   max_value;
};

namespace
{
const Event Space = Event::Character(' ');

const std::map<Base, BaseProperties> BasePropertiesByte {
    {Base::Decimal,     { Base::Decimal, 10, 3, {}, 255} },
    {Base::Octal,       { Base::Octal,   8,  3, "O", 255} },
    {Base::Hexadecimal, { Base::Hexadecimal, 16, 2, "$", 255} },
    {Base::Binary,      { Base::Binary,  2,  8, "%", 255} }
};

const std::map<Base, BaseProperties> BasePropertiesWord {
    {Base::Decimal,     { Base::Decimal, 10, 5, {}, 65535 } },
    {Base::Octal,       { Base::Octal,   8,  6, "O", 65535 } },
    {Base::Hexadecimal, { Base::Hexadecimal, 16, 4, "$", 65535 } },
    {Base::Binary,      { Base::Binary,  2,  16, "%", 65535 } }
};

int ChangeDigit(int current_value, int digit_number, int base, int delta, int max_value_of_representation)
{
    int p = static_cast<int>(std::pow(base, digit_number));

    // First, shift the wanted digit to the first position...
    int shifted_current_value = current_value / p;

    // Now clear all the higher digits...
    int zeroth_digit_value = shifted_current_value % base;

    // Apply the delta...
    int new_zeroth_value = zeroth_digit_value + delta;

    // Make the digit wrap within the base...
    int new_digit = (new_zeroth_value < 0) ? (new_zeroth_value + base) % base
                                             :
                                             new_zeroth_value % base;

    // Remove the previous value of the digit and then
    // replace it with the new one...
    int result = current_value - (zeroth_digit_value * p) + (new_digit * p);

    if ( result > max_value_of_representation )
    {
        // Recalculate
        int digit_value_of_max_value = (max_value_of_representation / p) % base;

        new_digit = new_digit % (digit_value_of_max_value + 1);
        result = current_value - (zeroth_digit_value * p) + (new_digit * p);
    }
    return result;
}

std::string GeneratePrefix(bool show_base, const BaseProperties &properties)
{
    if ( show_base )
        return properties.base_prefix;
    else
        return {};
}

std::string GenerateStringValue(Base base, int max_digits, int data)
{
    char buffer[16];

    switch (base)
    {
    case Base::Decimal:
        return std::string( buffer, snprintf(buffer, sizeof buffer, "%0*i", max_digits, data) );
        break;
    case Base::Hexadecimal:
        return std::string( buffer, snprintf(buffer, sizeof buffer, "%0*X", max_digits, data) );
        break;
    case Base::Octal:
        return std::string( buffer, snprintf(buffer, sizeof buffer, "%0*o", max_digits, data) );
        break;
    case Base::Binary:
        {
            std::to_chars_result result = std::to_chars( std::begin(buffer),
                                                         std::end(buffer),
                                                         data,
                                                         2);
            size_t len = std::distance(buffer, result.ptr);

            return std::string(max_digits, '0').replace(max_digits - len, len, buffer, len);
        }
        break;
    default:
        break;
    }
    return { };
}

Base NewBaseValue(int current_base_value, int delta)
{
    int limit_delta = delta % Base::END; // Wrap around
    int nonnegative_delta = limit_delta + Base::END; // >= 0

    return Base((current_base_value + nonnegative_delta) % Base::END); // Wrap the sum around
}

}

class InputByteBase : public ComponentBase {
public:
    struct KeyMap {
        Event increment_value_event    = Event::ArrowUp;
        Event decrement_value_event    = Event::ArrowDown;
        Event edit_mode_toggle_event   = Event::Return;
        Event increment_base_event     = Event::ArrowRight;
        Event decrement_base_event     = Event::ArrowLeft;
        Event toggle_base_prefix_event = Space;

        Event single_digit_edit_mode_toggle_event = Event::Character('d');
        Event increment_current_digit_event       = Event::ArrowLeft;
        Event decrement_current_digit_event       = Event::ArrowRight;
        Event increment_single_digit_value_event  = Event::ArrowUp;
        Event decrement_single_digit_value_event  = Event::ArrowDown;
    };

    InputByteBase(Ref<InputByteOption> option)
         :
         option_(std::move(option))
    {
    }

    void toggleEditMode() { *option_->edit_mode = !*option_->edit_mode; }
    void toggleSingleDigitEditMode() { *option_->single_digit_edit_mode = !*option_->single_digit_edit_mode; }

    // Component implementation:
    Element OnRender() override
    {
        bool is_focused = Focused();
        std::string v = GenerateStringValue( *option_->base, BasePropertiesByte.at(*option_->base).max_digits, *option_->data );
        std::string prefix = GeneratePrefix( *option_->base_prefix, BasePropertiesByte.at(*option_->base) );
        auto main_decorator = ftxui::size(HEIGHT, EQUAL, 1) |
                              ftxui::size(ftxui::WIDTH, EQUAL, static_cast<int>(v.length() + prefix.length()) );
        Element element;
        Element value_element = text( v );

        if ( is_focused )
            value_element |= focus;

        if ( hovered_ || (is_focused && !*option_->edit_mode) )
            value_element |= inverted;

        if ( *option_->edit_mode )
            element = hbox( { text( prefix ), value_element | color(Color::Yellow) | underlined } );
        else if ( *option_->single_digit_edit_mode )
        {
            // Break into 3 parts
            size_t  split_point = v.length() - *option_->current_digit - 1;
            Element first_part  = text( v.substr(0, split_point) ) | color( Color::Yellow ) | dim;
            Element cursor_part = text( v.substr(split_point, 1) ) | color(Color::Yellow);
            Element last_part   = text( v.substr(split_point + 1, v.length() - split_point - 1) ) | color(Color::Yellow) | dim;
            Element number_part = hbox( { first_part, cursor_part, last_part } );

            if ( is_focused )
                number_part |= focus;
            element = hbox( { text( prefix ),  number_part } );
        }
        else
            element = hbox( { text( prefix ), value_element } );

        element |= main_decorator;
        element |= reflect(box_);

        return element;
    }

    bool OnEvent(Event event) override
    {
        if (event == Event::Custom) {
            return false;
        }

        if ( event.is_mouse() ) {
            return OnMouseEvent(event);
        }

        if ( !Focused() )
            return false;

        if (event == keymap_.edit_mode_toggle_event)
        {
            if (!*option_->single_digit_edit_mode)
            toggleEditMode();
            return true;
        }

        if (event == keymap_.single_digit_edit_mode_toggle_event)
        {
            if (!*option_->edit_mode)
                toggleSingleDigitEditMode();
            return true;
        }

        if ( *option_->edit_mode )
            return OnEditModeEvent(event);
        else if ( *option_->single_digit_edit_mode )
            return OnSingleDigitEditModeEvent(event);

        return false;
    }

private:
    bool OnMouseEvent(Event event) {
        hovered_ = box_.Contain(event.mouse().x, event.mouse().y) && CaptureMouse(event);

        if (!hovered_)
            return false;

        if (event.mouse().button != Mouse::Left ||
            event.mouse().motion != Mouse::Pressed) {
            return false;
        }

        TakeFocus();
        return true;
    }

    bool OnEditModeEvent(Event &event)
    {
        if (event == keymap_.increment_value_event) // INCrement value
        {
            ++*option_->data;
            option_->on_change();
        }
        else if (event == keymap_.decrement_value_event) // DECrement value
        {
            --*option_->data;
            option_->on_change();
        }
        else if (event == keymap_.decrement_base_event)
            *option_->base = NewBaseValue(*option_->base, -1);
        else if (event == keymap_.increment_base_event)
            *option_->base = NewBaseValue(*option_->base, 1);
        else if (event == keymap_.toggle_base_prefix_event)
        {
            // Toggle the prefix
            *option_->base_prefix = !*option_->base_prefix;
        }
        return true;
    }

    bool OnSingleDigitEditModeEvent(Event &event)
    {
        if (event == keymap_.increment_current_digit_event)
            *option_->current_digit = NextDigit( *option_->current_digit );
        else if (event == keymap_.decrement_current_digit_event)
            *option_->current_digit = PreviousDigit( *option_->current_digit );
        else if (event == keymap_.increment_single_digit_value_event)
            *option_->data = ChangeDigit( *option_->data, *option_->current_digit, BasePropertiesByte.at(*option_->base).base_representation, 1, BasePropertiesByte.at(*option_->base).max_value);
        else if (event == keymap_.decrement_single_digit_value_event)
            *option_->data = ChangeDigit( *option_->data, *option_->current_digit, BasePropertiesByte.at(*option_->base).base_representation, -1, BasePropertiesByte.at(*option_->base).max_value);

        return true;
    }

    bool Focusable() const final { return true; }

    int  NextDigit(int digit_number)
    {
        return (digit_number == BasePropertiesByte.at(*option_->base).max_digits - 1) ? 0 : digit_number + 1;
    }

    int  PreviousDigit(int digit_number)
    {
        return (digit_number == 0) ? BasePropertiesByte.at(*option_->base).max_digits - 1 :
                                     digit_number - 1;
    }

    bool hovered_ = false;

    Box                  box_;
    Ref<InputByteOption> option_;
    KeyMap               keymap_;
};

class InputWordBase : public ComponentBase {
public:
    struct KeyMap {
        Event increment_value_event    = Event::ArrowUp;
        Event decrement_value_event    = Event::ArrowDown;
        Event edit_mode_toggle_event   = Event::Return;
        Event increment_base_event     = Event::ArrowRight;
        Event decrement_base_event     = Event::ArrowLeft;
        Event toggle_base_prefix_event = Space;

        Event single_digit_edit_mode_toggle_event = Event::Character('d');
        Event increment_current_digit_event       = Event::ArrowLeft;
        Event decrement_current_digit_event       = Event::ArrowRight;
        Event increment_single_digit_value_event  = Event::ArrowUp;
        Event decrement_single_digit_value_event  = Event::ArrowDown;
    };

    InputWordBase(Ref<InputWordOption> option)
        :
        option_(std::move(option))
    {
    }

    void toggleEditMode() { *option_->edit_mode = !*option_->edit_mode; }
    void toggleSingleDigitEditMode() { *option_->single_digit_edit_mode = !*option_->single_digit_edit_mode; }

    // Component implementation:
    Element OnRender() override
    {
        bool        is_focused = Focused();
        std::string v = GenerateStringValue( *option_->base, BasePropertiesWord.at(*option_->base).max_digits, *option_->data );
        std::string prefix = GeneratePrefix( *option_->base_prefix, BasePropertiesWord.at(*option_->base) );
        auto main_decorator = ftxui::size(HEIGHT, EQUAL, 1) |
                              ftxui::size(ftxui::WIDTH, EQUAL, static_cast<int>(v.length() + prefix.length()) );
        Element element;
        Element value_element = text( v );

        if ( is_focused )
            value_element |= focus;

        if ( hovered_ || (is_focused && !*option_->edit_mode) )
            value_element |= inverted;

        if ( *option_->edit_mode )
            element = hbox( { text( prefix ), value_element | color(Color::Yellow) | underlined } );
        else if ( *option_->single_digit_edit_mode )
        {
            // Break into 3 parts
            size_t  split_point = v.length() - *option_->current_digit - 1;
            Element first_part  = text( v.substr(0, split_point) ) | color( Color::Yellow ) | dim;
            Element cursor_part = text( v.substr(split_point, 1) ) | color(Color::Yellow);
            Element last_part   = text( v.substr(split_point + 1, v.length() - split_point - 1) ) | color(Color::Yellow) | dim;
            Element number_part = hbox( { first_part, cursor_part, last_part } );

            if ( is_focused )
                number_part |= focus;
            element = hbox( { text( prefix ), number_part } );
        }
        else
            element = hbox( { text( prefix ), value_element } );

        element |= main_decorator;
        element |= reflect(box_);

        return element;
    }

    bool OnEvent(Event event) override
    {
        if (event == Event::Custom)
            return false;

        if ( event.is_mouse() )
            return OnMouseEvent(event);

        if ( !Focused() )
            return false;

        if (event == keymap_.edit_mode_toggle_event)
        {
            if (!*option_->single_digit_edit_mode)
            toggleEditMode();
            return true;
        }

        if (event == keymap_.single_digit_edit_mode_toggle_event)
        {
            if (!*option_->edit_mode)
            toggleSingleDigitEditMode();
            return true;
        }

        if ( *option_->edit_mode )
            return OnEditModeEvent(event);
        else if ( *option_->single_digit_edit_mode )
            return OnSingleDigitEditModeEvent(event);

        return false;
    }
private:
    bool OnMouseEvent(Event event) {
        hovered_ = box_.Contain(event.mouse().x, event.mouse().y) && CaptureMouse(event);

        if (!hovered_) {
            return false;
        }

        if (event.mouse().button != Mouse::Left ||
            event.mouse().motion != Mouse::Pressed) {
            return false;
        }

        TakeFocus();
        return true;
    }

    bool OnEditModeEvent(Event &event)
    {
        if (event == keymap_.increment_value_event) // INCrement value
        {
            ++*option_->data;
            option_->on_change();
        }
        else if (event == keymap_.decrement_value_event) // DECrement value
        {
            --*option_->data;
            option_->on_change();
        }
        else if (event == keymap_.decrement_base_event)
            *option_->base = NewBaseValue(*option_->base, -1);
        else if (event == keymap_.increment_base_event)
            *option_->base = NewBaseValue(*option_->base, 1);
        else if (event == keymap_.toggle_base_prefix_event)
        {
            // Toggle the prefix
            *option_->base_prefix = !*option_->base_prefix;
        }
        return true;
    }

    bool OnSingleDigitEditModeEvent(Event &event)
    {
        if (event == keymap_.increment_current_digit_event)
            *option_->current_digit = NextDigit( *option_->current_digit );
        else if (event == keymap_.decrement_current_digit_event)
            *option_->current_digit = PreviousDigit( *option_->current_digit );
        else if (event == keymap_.increment_single_digit_value_event)
            *option_->data = ChangeDigit( *option_->data, *option_->current_digit, BasePropertiesWord.at(*option_->base).base_representation, 1, BasePropertiesWord.at(*option_->base).max_value);
        else if (event == keymap_.decrement_single_digit_value_event)
            *option_->data = ChangeDigit( *option_->data, *option_->current_digit, BasePropertiesWord.at(*option_->base).base_representation, -1, BasePropertiesWord.at(*option_->base).max_value);

        return true;
    }

    bool Focusable() const final { return true; }

    int  NextDigit(int digit_number)
    {
        return (digit_number == BasePropertiesWord.at(*option_->base).max_digits - 1) ? 0 : digit_number + 1;
    }

    int  PreviousDigit(int digit_number)
    {
        return (digit_number == 0) ? BasePropertiesWord.at(*option_->base).max_digits - 1 :
                                     digit_number - 1;
    }

    bool hovered_ = false;

    Box                  box_;
    Ref<InputWordOption> option_;
    KeyMap               keymap_;
};

class InputBitBase : public ComponentBase {
public:
    struct KeyMap {
        Event toggle_current_mask_event = Space;
    };

    InputBitBase(Ref<StatusOption> option, size_t our_mask)
        :
        option_( std::move(option) ),
        our_mask_(our_mask)
    {
    }

    Element OnRender() override
    {
        if ( our_mask_ == 0 )
            return vbox( _generateWidgets() | reflect(box_) ); // First one
        else
            return hbox({ filler(), vbox( _generateWidgets() | reflect(box_) ) }) | flex; // All others
    }

    bool Focusable() const final { return true; }

    bool OnEvent(Event event) override
    {
        if (event == Event::Custom)
            return false;

        if ( event.is_mouse() )
            return OnMouseEvent(event);

        if ( !Focused() )
            return false;

        if (event == keymap_.toggle_current_mask_event)
        {
            // Don't toggle if we don't have a mask...
            if ( mask().mask_value != StatusOption::Mask::NoMask )
            {
                option_().status() ^= mask().mask_value;
                option_().on_change();
                return true;
            }
        }
        return false;
    }

    bool OnMouseEvent(Event event)
    {
        hovered_ = box_.Contain( event.mouse().x, event.mouse().y );

        if (!hovered_) {
            return false;
        }

        if (event.mouse().button != Mouse::Left ||
            event.mouse().motion != Mouse::Pressed) {
            return false;
        }

        if ( mask().mask_value == StatusOption::Mask::NoMask )
            return false; // No mask present, so don't accept the selection attempt

        TakeFocus();
        return true;
    }

    const StatusOption::Mask &mask() const { return option_().masks[ our_mask_ ]; }

protected:
    bool                     hovered_ = false;
    ftxui::Ref<StatusOption> option_;
    size_t                   our_mask_ = 0;
    KeyMap                   keymap_;
    Box                      box_;

    Elements _generateWidgets() const
    {
        const char *set = "◉";
        const char *not_set = "○";
        const char *no_value = " ";
        bool has_mask = mask().mask_value != StatusOption::Mask::NoMask;
        bool mask_is_set = option_->status() & mask().mask_value;
        Element indicator;
        Element message = text(mask().what_to_display);

        if ( has_mask )
            if ( mask_is_set )
                indicator = text(set);
            else
                indicator = text(not_set);
        else
            indicator = text(no_value);

        if ( has_mask )
            message |= color( (mask_is_set) ? Color::Green : Color::Red ); // Only color if there is a mask

        if ( hovered_ || Focused() )
            message |= inverted;

        return { indicator, message };
    }
};

Component InputBit(Ref<StatusOption> option, size_t mask_number)
{
    return Make<InputBitBase>( std::move(option), mask_number);
}

Component InputByte(Ref<InputByteOption> option)
{
    return Make<InputByteBase>( std::move(option) );
}

Component InputWord(Ref<InputWordOption> option)
{
    return Make<InputWordBase>( std::move(option) );
}

Component InputStatus(Ref<StatusOption> option)
{
    Components children;

    children.reserve( option->masks.size() );
    for (size_t count = 0; count < option().masks.size(); ++count)
        children.emplace_back( InputBit(option, count) );

    return Container::Horizontal( std::move(children) );
}
