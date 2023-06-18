#ifndef INPUTNUMBER_HPP
#define INPUTNUMBER_HPP

#include <ftxui/component/component.hpp>
#include <functional>

enum Base {
    Binary,
    Octal,
    Decimal,
    Hexadecimal,
    END = Hexadecimal
};

struct InputByteOption {
  /// Called when the content changes.
  std::function<void()> on_change = [] {};

  ftxui::Ref<uint8_t> data;
  ftxui::Ref<Base>    base = Base::Hexadecimal;
  ftxui::Ref<bool>    base_prefix = false;
  ftxui::Ref<bool>    edit_mode = false;
  ftxui::Ref<bool>    single_digit_edit_mode = false;
  ftxui::Ref<int>     current_digit = 0;
};

struct InputWordOption {
  /// Called when the content changes.
  std::function<void()> on_change = [] {};

  ftxui::Ref<uint16_t> data;
  ftxui::Ref<Base>     base = Base::Hexadecimal;
  ftxui::Ref<bool>     base_prefix = false;
  ftxui::Ref<bool>     edit_mode = false;
  ftxui::Ref<bool>     single_digit_edit_mode = false;
  ftxui::Ref<int>      current_digit = 0;
};

ftxui::Component InputByte(ftxui::Ref<InputByteOption> option = {});
ftxui::Component InputWord(ftxui::Ref<InputWordOption> option = {});

#endif // INPUTNUMBER_HPP
