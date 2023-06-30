#ifndef INPUTNUMBER_HPP
#define INPUTNUMBER_HPP

#include <ftxui/component/component.hpp>
#include <functional>
#include <map>

enum Base {
    Binary,
    Octal,
    Decimal,
    Hexadecimal,
    END = Hexadecimal + 1
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

struct StatusOption {
  struct Mask {
      int         mask_value;
      std::string what_to_display;
  };

  /// Called when the content changes.
  std::function<void()> on_change = [] {};
  std::function<void()> on_current_mask_change = [] {};

  ftxui::Ref<int>   status;
  ftxui::Ref<int>   current_mask = 0;
  std::vector<Mask> masks;
};

ftxui::Component InputByte(ftxui::Ref<InputByteOption> option = {});
ftxui::Component InputWord(ftxui::Ref<InputWordOption> option = {});
ftxui::Component InputStatus(ftxui::Ref<StatusOption> option = {});

#endif // INPUTNUMBER_HPP
