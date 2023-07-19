#include "list.hpp"
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

namespace
{

// Similar to std::clamp, but allow hi to be lower than lo.
template <class T>
constexpr const T& myclamp(const T& v, const T& lo, const T& hi)
{
  return v < lo ? lo : hi < v ? hi : v;
}

}

class ListBrowser : public ComponentBase {
public:
    ListBrowser(ConstStringListRef entries, int *selected, Ref<ListOption> option)
    :
    entries_( entries ),
    selected_( selected ),
    option_( std::move(option) )
    {
    }

    Element Render() override
    {
        return vbox( generateItemElements() ) | vscroll_indicator | yframe | reflect(box_);
    }

    bool OnEvent(Event event) override
    {
        Clamp();
        if (!CaptureMouse(event)) {
            return false;
        }

        if (event.is_mouse()) {
            return OnMouseEvent(event);
        }

        if (Focused()) {
            const int old_hovered = hovered_;

            if (event == Event::ArrowUp || event == Event::Character('k')) {
                (hovered_)--;
            }
            else if (event == Event::ArrowDown || event == Event::Character('j')) {
                (hovered_)++;
            }
            else if (event == Event::PageUp) {
                (hovered_) -= box_.y_max - box_.y_min;
            }
            else if (event == Event::PageDown) {
                (hovered_) += box_.y_max - box_.y_min;
            }
            else if (event == Event::Home) {
                (hovered_) = 0;
            }
            else if (event == Event::End) {
                (hovered_) = size() - 1;
            }
            else if (event == Event::Tab && size()) {
                hovered_ = (hovered_ + 1) % size();
            }
            else if (event == Event::TabReverse && size()) {
                hovered_ = (hovered_ + size() - 1) % size();
            }

            hovered_ = myclamp(hovered_, 0, size() - 1);

            if (hovered_ != old_hovered) {
                focused_entry() = hovered_;
                option_->on_focused_entry_changed();
                return true;
            }
        }

        if (event == Event::Character(' ') || event == Event::Return) {
            *selected_ = hovered_;
            option_->on_item_selected();
            return true;
        }

        return false;
    }

    bool OnMouseEvent(Event event)
    {
        if (event.mouse().button == Mouse::WheelDown ||
            event.mouse().button == Mouse::WheelUp) {
            return OnMouseWheel(event);
        }

        for (int i = 0; i < size(); ++i) {
            if ( !boxes_[i].Contain(event.mouse().x, event.mouse().y) ) {
                continue;
            }

            TakeFocus();
            focused_entry() = i;
            if (event.mouse().button == Mouse::Left &&
                event.mouse().motion == Mouse::Released) {
                if ( *selected_ != i ) {
                    *selected_ = i;
                    option_->on_item_selected();
                }

                return true;
            }
        }
        return false;
    }

    bool OnMouseWheel(Event event)
    {
        if (!box_.Contain(event.mouse().x, event.mouse().y)) {
            return false;
        }

        const int old_hovered = hovered_;

        if (event.mouse().button == Mouse::WheelUp) {
            (hovered_)--;
        }
        if (event.mouse().button == Mouse::WheelDown) {
            (hovered_)++;
        }

        hovered_ = myclamp(hovered_, 0, size() - 1);

        if (hovered_ != old_hovered) {
            option_->on_focused_entry_changed();
        }

        return true;
    }

protected:
    ConstStringListRef  entries_;
    int                *selected_;
    int                 hovered_ = *selected_;
    std::vector<Box>    boxes_;
    Box                 box_;
    Ref<ListOption>     option_;


    void Clamp()
    {
        boxes_.resize(size());
        *selected_      = myclamp(*selected_, 0, size() - 1);
        focused_entry() = myclamp(focused_entry(), 0, size() - 1);
        hovered_        = myclamp(hovered_, 0, size() - 1);
    }

    bool  Focusable() const final { return entries_.size(); }
    int  &focused_entry() { return option_->focused_entry(); }
    int   size() const { return int(entries_.size()); }

    Elements generateItemElements()
    {
        Clamp();
        Elements elements;
        const bool is_menu_focused = Focused();

        elements.reserve( size() );
        for (int i = 0; i < size(); ++i) {
          const bool is_focused = (focused_entry() == i) && is_menu_focused;
          const bool is_selected = (hovered_ == i);
          auto focus_management = !is_selected      ? nothing
                                  : is_menu_focused ? focus
                                                    : select;
          auto state = EntryState{
              entries_[i],
              *selected_ == i,
              is_selected,
              is_focused,
          };
          auto element =
              (option_->transform ? option_->transform
                                  : ListOption::Simple().transform)(state);

          elements.push_back(element | focus_management | reflect(boxes_[i]));
        }
        return elements;
    }
};

ListOption ListOption::Simple()
{
    auto option = ListOption();

    option.transform = [](const EntryState &s) {
        auto t = text(s.label);

        if (s.active) {
            t |= bold;
        }
        if (s.focused) {
            t |= inverted;
        }
        return t;
    };
    return option;
}


Component List(ConstStringListRef entries, int * selected, Ref<ListOption> option)
{
    return Make<ListBrowser>( entries, selected, std::move(option) );
}
