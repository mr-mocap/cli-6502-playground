#include "inputnumber.hpp"
#include <bitset>
#include <map>
#include <cmath>
#include <numeric>

#include "ftxui/dom/elements.hpp"

using namespace ftxui;

struct BaseProperties {
  InputByteOption::Base base;
  int                   base_representation;
  int                   max_digits;
  std::string           base_prefix;
  static constexpr int  max_value = static_cast<int>(std::numeric_limits<uint8_t>::max());
};

namespace
{

int ChangeDigit(int current_value, int digit_number, int base, int delta, int max_value_of_representation)
{
    int p = pow(base, digit_number);

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

std::string GenerateStringValue(InputByteOption::Base base, uint8_t data)
{
    char buffer[16];

    switch (base)
    {
    case InputByteOption::Base::Decimal:
        return std::string( buffer, snprintf(buffer, sizeof buffer, "%03i", data) );
        break;
    case InputByteOption::Base::Hexadecimal:
        return std::string( buffer, snprintf(buffer, sizeof buffer, "%02X", data) );
        break;
    case InputByteOption::Base::Octal:
        return std::string( buffer, snprintf(buffer, sizeof buffer, "%03o", data) );
        break;
    case InputByteOption::Base::Binary:
        return std::bitset<8>(data).to_string();
        break;
    default:
        break;
    }
    return { };
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
    Element Render() override
    {
      bool is_focused = Focused();
      std::string v = GenerateStringValue( *option_->base, *option_->data );
      std::string prefix = GeneratePrefix( *option_->base_prefix, base_properties_[*option_->base] );
      auto main_decorator = ftxui::size(HEIGHT, EQUAL, 1) |
                            ftxui::size(ftxui::WIDTH, EQUAL, v.length() + prefix.length() );
      Element element;

      if ( *option_->edit_mode )
          element = hbox( { text( prefix ), color(Color::Green, text( v )) } );
      else if ( *option_->single_digit_edit_mode )
      {
          // Break into 3 parts
          size_t  split_point = v.length() - *option_->current_digit - 1;
          Element first_part  = text( v.substr(0, split_point) ) | color( Color::Yellow );
          Element cursor_part = text( v.substr(split_point, 1) ) |  bgcolor(Color::White) | color(Color::Red);
          Element last_part   = text( v.substr(split_point + 1, v.length() - split_point - 1) ) | color(Color::Yellow);

          element = hbox( { text( prefix ), first_part, cursor_part, last_part } );
      }
      else
          element = hbox( { text( prefix ), text( v ) } );

      element |= main_decorator;
      element |= reflect(box_);

      if ( is_focused )
          element |= focus;

      if ( hovered_ || is_focused )
          element |= inverted;

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
        {
          if (*option_->base == InputByteOption::Base::Decimal) // Check if we need to wrap around
            *option_->base = InputByteOption::Base::Binary;
          else
          {
            unsigned int value = static_cast<unsigned int>(*option_->base);

            *option_->base = static_cast<InputByteOption::Base>(value - 1);
          }
        }
        else if (event == keymap_.increment_base_event)
        {
          unsigned int value = static_cast<int>(*option_->base);

          *option_->base = static_cast<InputByteOption::Base>((value + 1) % 4);
        }
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
        {
            *option_->current_digit = NextDigit( *option_->current_digit );
        }
        else if (event == keymap_.decrement_current_digit_event)
        {
            *option_->current_digit = PreviousDigit( *option_->current_digit );
        }
        else if (event == keymap_.increment_single_digit_value_event)
        {
            *option_->data = ChangeDigit( *option_->data, *option_->current_digit, base_properties_[*option_->base].base_representation, 1, base_properties_[*option_->base].max_value);
        }
        else if (event == keymap_.decrement_single_digit_value_event)
        {
            *option_->data = ChangeDigit( *option_->data, *option_->current_digit, base_properties_[*option_->base].base_representation, -1, base_properties_[*option_->base].max_value);
        }

        return true;
    }

    bool Focusable() const final { return true; }

    int  NextDigit(int digit_number)
    {
        return (digit_number == base_properties_[*option_->base].max_digits - 1) ? 0 : digit_number + 1;
    }

    int  PreviousDigit(int digit_number)
    {
        return (digit_number == 0) ? base_properties_[*option_->base].max_digits - 1 :
                                     digit_number - 1;
    }

    bool hovered_ = false;

    Box                  box_;
    Ref<InputByteOption> option_;
    KeyMap               keymap_;
    std::map<InputByteOption::Base, BaseProperties> base_properties_{
      {InputByteOption::Base::Decimal,     { InputByteOption::Base::Decimal, 10, 3} },
      {InputByteOption::Base::Octal,       { InputByteOption::Base::Octal,   8,  3, "O"} },
      {InputByteOption::Base::Hexadecimal, { InputByteOption::Base::Hexadecimal, 16, 2, "$"} },
      {InputByteOption::Base::Binary,      { InputByteOption::Base::Binary,  2,  8, "%"} }
    };

    static const Event Space;
    static const Event Ctrl;
};

const Event InputByteBase::Space = Event::Character(' ');

class InputWordBase : public ComponentBase {
 public:
    InputWordBase(Ref<uint16_t> content, Ref<InputWordOption> option)
         :
         content_(std::move(content)),
         option_(std::move(option))
    {
    }

    void toggleEditMode() { *option_->edit_mode = !*option_->edit_mode; }

  // Component implementation:
  Element Render() override
  {
    char buffer[4 + 1];
    bool is_focused = Focused();
    auto main_decorator = ftxui::size(HEIGHT, EQUAL, 1) | ftxui::size(ftxui::WIDTH, EQUAL, 4);

    snprintf(buffer, sizeof buffer, "%04X", *content_);
    auto element = text( std::string(buffer, 4) ) | main_decorator | reflect(box_);

    if ( is_focused )
        element |= focus;

    if ( *option_->edit_mode )
        element = color(Color::DarkOliveGreen1, element);

    if ( hovered_ || is_focused )
        element |= inverted;

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

    if (event == Event::Return)
    {
        toggleEditMode();
        return true;
    }

    if ( *option_->edit_mode )
        return OnEditModeEvent(event);

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
      if (event == Event::ArrowRight)
      {
          ++*content_;
          option_->on_change();
      }
      else if (event == Event::ArrowLeft)
      {
          --*content_;
          option_->on_change();
      }
      return true;
  }

  bool Focusable() const final { return true; }

  bool hovered_ = false;
  Ref<uint16_t>  content_;

  Box box_;
  Ref<InputWordOption> option_;
};

Component InputByte(Ref<InputByteOption> option)
{
    return Make<InputByteBase>( std::move(option) );
}

Component InputWord(Ref<uint16_t> content, Ref<InputWordOption> option)
{
    return Make<InputWordBase>( std::move(content), std::move(option) );
}
