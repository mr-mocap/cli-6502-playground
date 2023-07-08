#include "pageview.hpp"
#include "ftxui/screen/color.hpp"
#include "ftxui/util/ref.hpp"
#include "ftxui/dom/elements.hpp"
#include <utility>
#include <array>
#include <iterator>
#include <algorithm>
#include <cstdio>

using namespace ftxui;

constexpr int DisplayCellsOfByte    = 2; // in hex characters.  2 hex digits
constexpr int DisplayCellsOfAddress = 4; // 16 bits (4 hex chars)
constexpr int DisplayCellsOfDecoratedAddress = 6; // '$' prefix.  ':' suffix
//constexpr int DisplayBytesPerLine   = 16;
//constexpr int LineNumberOfCells     = DisplayCellsOfAddress + 2 + DisplayBytesPerLine * DisplayCellsOfByte;
constexpr int LinesInDisplay        = 16;
constexpr int Spaces = 1; // Number of spaces in front of a displayed byte
constexpr int AddressPart = DisplayCellsOfDecoratedAddress;
constexpr int DataPart = DisplayCellsOfByte * 16 + 16 * Spaces;

namespace
{

int AddressOfLine(int page, int linenumber)
{
    return ((page << 8) + (16 * linenumber)) & 0xFFFF;
}

std::string AddressAsString(int page_number, int line)
{
    int address = AddressOfLine(page_number, line);
    char buffer[AddressPart + 1];

    size_t num_written = snprintf(buffer, sizeof(buffer), "$%.4X:", static_cast<unsigned int>(address));

    return { buffer, num_written };
}

std::string PageNumberAsString(int page_number)
{
    char buffer[4];

    snprintf(buffer, sizeof(buffer), "%02X ", page_number);
    return { buffer, 2 };
}

}

class PageView : public Node
{
public:
    PageView(std::shared_ptr<RamBusDeviceView> model,
             Ref<int>  current_byte,
             Ref<int>  program_counter,
             bool      in_edit_mode);

    void ComputeRequirement() override;

    void Render(Screen &screen) override;

    void SetBox(Box box) override;
protected:
    std::shared_ptr<RamBusDeviceView> _model;
    Ref<int>                          _current_byte_in_page;
    Ref<int>                          _program_counter;
    Element                           _display;
    bool                              _in_edit_mode;

    std::string byteAsString(int line, int column) const;

    unsigned int memoryAt(int line, int column) const
    {
        size_t page_index = _model->page() * 256;
        size_t line_index = page_index + line * 16;
        size_t index = line_index + static_cast<size_t>(column);

        return _model->model()->memory()[ index ];
    }

    Elements generateLineWidgets(int line) const
    {
        Elements row;

        row.reserve(17);
        row.emplace_back( notflex( text( AddressAsString(_model->page(), line) ) ) );
        for (int column = 0; column < 16; ++column)
        {
            row.emplace_back( notflex( text(" ") ) );
            row.emplace_back( notflex( text( byteAsString(line, column) ) ) );
        }
        return row;
    }

    void affectCurrentByteInEditMode(Screen &screen, const Box &box);
    void affectCurrentByteInNonEditMode(Screen &screen, const Box &box);
    void affectCurrentProgramCounter(Screen &screen, const Box &box);
};

PageView::PageView(std::shared_ptr<RamBusDeviceView> model,
                   Ref<int>       current_byte,
                   Ref<int>       show_pc,
                   bool           in_edit_mode)
    :
    _model{ std::move(model) },
    _current_byte_in_page{ std::move(current_byte) },
    _program_counter{ std::move(show_pc) },
    _in_edit_mode{ in_edit_mode }
{
    std::vector<Elements> grid_rows;

    grid_rows.reserve(16);
    for (int line = 0; line < 16; ++line)
        grid_rows.emplace_back( generateLineWidgets(line) );

    if ( _in_edit_mode )
    {
        children_.emplace_back( window( hbox({ text(" Memory Page: "), text( PageNumberAsString(_model->page()) ) }),
                                        dim( gridbox( std::move(grid_rows) ) ) ) );
    }
    else
    {
        children_.emplace_back( window( hbox({ text(" Memory Page: "), text( PageNumberAsString(_model->page()) ) }),
                                        gridbox( std::move(grid_rows) ) ) );
    }
}

void PageView::ComputeRequirement()
{
    Node::ComputeRequirement();
    requirement_ = children_[0]->requirement();
}

void PageView::SetBox(Box box)
{
    Node::SetBox(box);
    children_[0]->SetBox(box);
}

void PageView::Render(Screen &screen)
{
    Node::Render(screen);

    // Show the PC if necessary...
    if (_current_byte_in_page() != -1)
    {
        if ( _in_edit_mode )
            affectCurrentByteInEditMode( screen, pageview_ui_box_for_byte( _current_byte_in_page() ) );
        else
            affectCurrentByteInNonEditMode( screen, pageview_ui_box_for_byte( _current_byte_in_page() ) );
    }

    if (_program_counter() != -1)
    {
        if ( (_program_counter() & 0xFF00) == (_model->page() << 8) )
        {
            // The program counter is in this page
            affectCurrentProgramCounter( screen, pageview_ui_box_for_byte( _program_counter() & 0xFF ) );
        }
    }
}

std::string PageView::byteAsString(int line, int column) const
{
    char buffer[4];
    size_t  num_written = snprintf(buffer, sizeof(buffer), "%02X", memoryAt(line, column));

    return { buffer, num_written };
}

void PageView::affectCurrentByteInEditMode(Screen &screen, const Box &cursor)
{
    // Translate to global coords and draw...
    Pixel &pixel1 = screen.PixelAt( box_.x_min + cursor.x_min    , box_.y_min + cursor.y_min );
    Pixel &pixel2 = screen.PixelAt( box_.x_min + cursor.x_min + 1, box_.y_min + cursor.y_min );

    pixel1.underlined = true;
    pixel1.foreground_color = Color::Yellow;
    pixel1.dim = false;
    pixel2.underlined = true;
    pixel2.foreground_color = Color::Yellow;
    pixel2.dim = false;
}

void PageView::affectCurrentByteInNonEditMode(Screen &screen, const Box &cursor)
{
    // Translate to global coords and draw...
    screen.PixelAt( box_.x_min + cursor.x_min    , box_.y_min + cursor.y_min ).inverted = true;
    screen.PixelAt( box_.x_min + cursor.x_min + 1, box_.y_min + cursor.y_min ).inverted = true;
}

void PageView::affectCurrentProgramCounter(Screen &screen, const Box &pc)
{
    // Translate to global coords and draw...
    screen.PixelAt( box_.x_min + pc.x_min    , box_.y_min + pc.y_min ).background_color = Color::Blue;
    screen.PixelAt( box_.x_min + pc.x_min + 1, box_.y_min + pc.y_min ).background_color = Color::Blue;
}

Element pageview(std::shared_ptr<RamBusDeviceView> model, Ref<int> current_byte, Ref<int> program_counter, bool in_edit_mode)
{
    return std::make_shared<PageView>( std::move(model), std::move(current_byte), std::move(program_counter), in_edit_mode );
}

int pageview_byte(int x_coord, int y_coord)
{
    // Coords relative to Element position
    if ( (y_coord < LinesInDisplay) && (x_coord < (AddressPart + DataPart)) )
    {
        // It is in our display...
        // Now figure out if the x-coord makes any sense...
        if (x_coord > AddressPart)
        {
            int pos_in_byte = (x_coord - AddressPart) % 3;

            if (pos_in_byte > 0)
            {
                // It does!
                return (y_coord << 4) | ((x_coord - AddressPart) / 3);
            }
        }
    }

    return -1; // No corresponding byte
}

Box pageview_ui_box_for_byte(int byte)
{
    int line_pos = byte & 0x0F;
    int line = (byte >> 4) & 0x0F;
    int x_pos = AddressPart + line_pos * 3 + 1;

    return Box{ x_pos + 1, x_pos + 2, line + 1, line + 1 };
}
