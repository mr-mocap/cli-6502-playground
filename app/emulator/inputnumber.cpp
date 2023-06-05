#include "inputnumber.hpp"
#include <bitset>
#include <map>

#include "ftxui/dom/elements.hpp"

using namespace ftxui;

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

    struct BaseProperties {
      InputByteOption::Base base;
      int                   max_digits;
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
      char buffer[16];
      int  number_written = 0;
      bool is_focused = Focused();

      switch (*option_->base)
      {
        case InputByteOption::Base::Decimal:
          // Let's don't show any prefix
          number_written = snprintf(buffer, sizeof buffer, "%03i", *option_->data);
          break;
        case InputByteOption::Base::Hexadecimal:
          if (*option_->base_prefix)
            number_written = snprintf(buffer, sizeof buffer, "$%02X", *option_->data);
          else
            number_written = snprintf(buffer, sizeof buffer, "%02X", *option_->data);
          break;
        case InputByteOption::Base::Octal:
          if (*option_->base_prefix)
            number_written = snprintf(buffer, sizeof buffer, "O%03o", *option_->data);
          else
            number_written = snprintf(buffer, sizeof buffer, "%03o", *option_->data);
          break;
        case InputByteOption::Base::Binary:
          {
            std::string s = std::bitset<8>(*option_->data).to_string();
            char *buf_ptr = buffer;

            if (*option_->base_prefix)
              *buf_ptr++ = '%';

            s.copy(buf_ptr, std::string::npos);
            buf_ptr += s.size();
            *buf_ptr = '\0';
            number_written = (buf_ptr - buffer);
          }
          break;
        default:
          break;
      }
      auto main_decorator = ftxui::size(HEIGHT, EQUAL, 1) | ftxui::size(ftxui::WIDTH, EQUAL, number_written);
      Element element;

      if ( *option_->edit_mode )
          element = color(Color::Green, text( std::string(buffer, number_written ) ));
      else if ( *option_->single_digit_edit_mode )
      {
          if ( *option_->current_digit == 0 )
          {
              // Break into only 2 parts
              Element first_part  = text( std::string(buffer, number_written - 1) ) | color(Color::Yellow);
              Element cursor_part = text( std::string(buffer + number_written - 1, 1) ) | focusCursorBlockBlinking | bgcolor(Color::White) | color(Color::Red);

              element = hbox( { first_part, cursor_part } );
          }
          else
          {
              // Break into 3 parts
              int     split_point = number_written - *option_->current_digit;
              Element first_part  = text( std::string(buffer, split_point - 1) ) | color(Color::Yellow);
              Element cursor_part = text( std::string(buffer + split_point, 1) ) | focusCursorBlockBlinking | bgcolor(Color::White) | color(Color::Red);
              Element last_part   = text( std::string(buffer + split_point + 1, number_written - split_point) ) | color(Color::Yellow);

              element = hbox( { first_part, cursor_part, last_part } );
          }
      }
      else
          element = text( std::string(buffer, number_written) );

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
        }
        else if (event == keymap_.decrement_single_digit_value_event)
        {
        }

        return true;
    }

    bool Focusable() const final { return true; }

    int  NextDigit(int digit_number)
    {
        return (digit_number == base_properties_[*option_->base].max_digits) ? 0 : digit_number + 1;
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
      {InputByteOption::Base::Decimal,     { InputByteOption::Base::Decimal, 3} },
      {InputByteOption::Base::Octal,       { InputByteOption::Base::Octal,   3} },
      {InputByteOption::Base::Hexadecimal, { InputByteOption::Base::Hexadecimal, 2} },
      {InputByteOption::Base::Binary,      { InputByteOption::Base::Binary,  8} }
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
