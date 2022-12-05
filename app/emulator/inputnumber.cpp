#include "inputnumber.hpp"

using namespace ftxui;

class InputByteBase : public ComponentBase {
 public:
    InputByteBase(Ref<uint8_t> content, Ref<InputByteOption> option)
         :
         content_(std::move(content)),
         option_(std::move(option))
    {
    }

    void toggleEditMode() { *option_->edit_mode = !*option_->edit_mode; }

  // Component implementation:
  Element Render() override
  {
    char buffer[2 + 1];
    bool is_focused = Focused();
    auto main_decorator = ftxui::size(HEIGHT, EQUAL, 1) | ftxui::size(ftxui::WIDTH, EQUAL, 2);

    snprintf(buffer, sizeof buffer, "%02X", *content_);
    auto element = text( std::string(buffer, 2) ) | main_decorator | reflect(box_);

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
  Ref<uint8_t>  content_;

  Box box_;
  Ref<InputByteOption> option_;
};

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

Component InputByte(Ref<uint8_t> content, Ref<InputByteOption> option)
{
    return Make<InputByteBase>( std::move(content), std::move(option) );
}

Component InputWord(Ref<uint16_t> content, Ref<InputWordOption> option)
{
    return Make<InputWordBase>( std::move(content), std::move(option) );
}
