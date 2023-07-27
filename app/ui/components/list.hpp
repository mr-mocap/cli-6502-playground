#ifndef LIST_HPP
#define LIST_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/util/ref.hpp>
#include <vector>
#include <string>


struct ListOption
{
    static ListOption Simple();

    // Style:
    std::function<ftxui::Element (const ftxui::EntryState &)> transform;

    /// Called when the selected entry changes.
    std::function<void()> on_focused_entry_changed = [] {};
    std::function<void()> on_item_selected  = [] {};

    ftxui::Ref<int> focused_entry = 0;
};

ftxui::Component List(ftxui::ConstStringListRef entries, int *selected, ftxui::Ref<ListOption> option);

#endif // LIST_HPP
