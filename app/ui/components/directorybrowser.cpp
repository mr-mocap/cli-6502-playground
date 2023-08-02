#include "directorybrowser.hpp"
#include <ftxui/dom/elements.hpp>
#include "list.hpp"
#include <QDebug>
#include <memory>
#include <vector>


using namespace ftxui;
using namespace std;

class DirectoryBrowser : public ComponentBase {
public:
    using DirEntryComparator = std::function<bool (const filesystem::directory_entry &, const filesystem::directory_entry &)>;

    DirectoryBrowser(Ref<InputDirectoryOption> option)
        :
        option_( std::move(option) ),
        list( List(&files_in_dir, &currently_selected_file_in_dir, &list_option_) ),
        ok_button( Button("Select", std::bind( &DirectoryBrowser::onSelectButtonPressed, this ), ButtonOption::Border() ) ),
        cancel_button( Button("Cancel", std::bind( &DirectoryBrowser::onCancelButtonPressed, this ), ButtonOption::Border() ) )
    {
        Add( Container::Vertical({
                    list,
                    Container::Horizontal({ ok_button, cancel_button })
                    })
            );
        list_option_.on_focused_entry_changed = std::bind( &DirectoryBrowser::onListFocusedEntryChanged, this );
        list_option_.on_item_selected = std::bind( &DirectoryBrowser::onListItemSelected, this );
    }

    /* Generates an internal list of files in the directory of the option */
    void synchronizeWithOption()
    {
        auto default_dir_entry_comparator = [](const filesystem::directory_entry &left,
                                               const filesystem::directory_entry &right)
        {
            if ( left.is_directory() != right.is_directory() )
                return left.is_directory();
            
            return left.path().filename() < right.path().filename();
        };

        files_in_dir.clear();
        files_in_dir.emplace_back("..");

        std::vector<filesystem::directory_entry> entries;

        for (auto const &dir_entry : filesystem::directory_iterator( option_->curent_directory() ))
        {
            if ( dir_entry.path().filename().string().front() == '.' )
            {
                if ( option_->show_hidden_files() )
                    entries.push_back( dir_entry );
            }
            else
                entries.push_back( dir_entry );
        }
        
        // Sort the entries
        std::sort( entries.begin(), entries.end(), default_dir_entry_comparator );

        // Add the entries...
        for (auto const &dir_entry : entries)
        {
            if ( dir_entry.is_directory() )
                files_in_dir.push_back( dir_entry.path().filename().string() + '/' );
            else
                files_in_dir.push_back( dir_entry.path().filename().string() );
        }
    }

    // Component implementation:
    Element Render() override
    {
        return window( text("Load File") | hcenter,
                       vbox({
                            hbox({ vbox({ filler(), text("Current Path") | size(WIDTH, EQUAL, 12), filler() }) | size(HEIGHT, EQUAL, 3),
                                   filler() | size(WIDTH, EQUAL, 1),
                                   text(option_->curent_directory->string()) | size(HEIGHT, EQUAL, 1) | xflex | border }) | size(HEIGHT, EQUAL, 3),
                            list->Render() | border | yflex | xflex,
                            hbox({ vbox({ filler(), text("Filename") | size(WIDTH, EQUAL, 8), filler() }) | size(HEIGHT, EQUAL, 3), filler() | size(WIDTH, EQUAL, 1), (currently_selected_file_in_dir == -1) ? text("") : text( files_in_dir[currently_selected_file_in_dir] ) | size(HEIGHT, EQUAL, 1) | border | xflex }) | size(HEIGHT, EQUAL, 3),
                            separatorDouble(),
                            hbox({ filler(), ok_button->Render() | notflex, filler(), cancel_button->Render() | notflex, filler() })
                            })
                      );
    }

protected:
    void onListItemSelected()
    {
        filesystem::path selected_item = files_in_dir[ currently_selected_file_in_dir ];
        filesystem::path new_path = option_->curent_directory() / selected_item;

        if ( filesystem::is_directory( new_path ) )
        {
            option_->curent_directory = filesystem::canonical( new_path );
            option_->curent_file->clear();
            synchronizeWithOption();
        }
        else if ( filesystem::is_regular_file( new_path ) )
        {
            option_->curent_file = std::move(selected_item);
            SetActiveChild( ok_button );
            ok_button->TakeFocus();
        }
    }

    void onListFocusedEntryChanged()
    {
    }

    void onSelectButtonPressed()
    {
        option_->finished( InputDirectoryOption::Ok );
    }

    void onCancelButtonPressed()
    {
        option_->finished( InputDirectoryOption::Cancel );
    }

    Elements generateFileListElements()
    {
        Elements elements;

        elements.reserve( files_in_dir.size() );
        for (auto const &filename : files_in_dir )
            elements.emplace_back( text(filename) );
        return elements;
    }

    Box                       box_;
    Ref<InputDirectoryOption> option_;
    ListOption                list_option_;
    Component                 list;
    Component                 ok_button;
    Component                 cancel_button;
    Component                 file_list;
    vector<string>            files_in_dir;
    int                       currently_selected_file_in_dir = -1;
};

Component InputDirectoryBrowser(Ref<InputDirectoryOption> option)
{
    return Make<DirectoryBrowser>( std::move(option) );
}

void UpdateDirectoryBrowser(Component directory_browser_component)
{
    auto directory_browser = dynamic_pointer_cast<DirectoryBrowser>(directory_browser_component);

    if ( directory_browser )
    {
        directory_browser->synchronizeWithOption();
    }
}
