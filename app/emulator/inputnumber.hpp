#ifndef INPUTNUMBER_HPP
#define INPUTNUMBER_HPP

#include <ftxui/component/component.hpp>
#include <functional>

struct InputByteOption {
  /// Called when the content changes.
  std::function<void()> on_change = [] {};

  ftxui::Ref<bool> edit_mode = false;
};

struct InputWordOption {
  /// Called when the content changes.
  std::function<void()> on_change = [] {};

  ftxui::Ref<bool> edit_mode = false;
};

ftxui::Component InputByte(ftxui::Ref<uint8_t> content, ftxui::Ref<InputByteOption> option = {});
ftxui::Component InputWord(ftxui::Ref<uint16_t> content, ftxui::Ref<InputWordOption> option = {});

#endif // INPUTNUMBER_HPP
