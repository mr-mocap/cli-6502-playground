#ifndef DIRECTORYBROWSER_HPP
#define DIRECTORYBROWSER_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/util/ref.hpp>
#include <filesystem>

struct InputDirectoryOption {
    static constexpr int Cancel = 0;
    static constexpr int Ok = 1;

    std::function<void(int)> finished; // Button Pressed as parameter

    ftxui::Ref<std::filesystem::path> curent_directory;
    ftxui::Ref<std::filesystem::path> curent_file;
    ftxui::Ref<bool>                  show_hidden_files = false;
};

ftxui::Component InputDirectoryBrowser(ftxui::Ref<InputDirectoryOption> option);

void UpdateDirectoryBrowser(ftxui::Component directory_browser);

#endif // DIRECTORYBROWSER_HPP
