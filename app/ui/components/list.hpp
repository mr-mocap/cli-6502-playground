#ifndef LIST_HPP
#define LIST_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/util/ref.hpp>
#include <vector>
#include <string>

using ftxui::Element;
using ftxui::EntryState;
using ftxui::Ref;
using ftxui::ConstStringListRef;

struct ListOption
{
    static ListOption Simple();

    // Style:
    std::function<Element (const EntryState &)> transform;

    /// Called when the selected entry changes.
    std::function<void()> on_focused_entry_changed = [] {};
    std::function<void()> on_item_selected  = [] {};

    Ref<int> focused_entry = 0;
};

ftxui::Component List(ConstStringListRef entries, int *selected, Ref<ListOption> option);

#endif // LIST_HPP
