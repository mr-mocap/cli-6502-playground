#include "pageview.hpp"
#include "ftxui/screen/color.hpp"
#include "ftxui/util/ref.hpp"
#include <utility>
#include <array>
#include <cstdio>

using namespace ftxui;

constexpr int DisplayCellsOfByte    = 2; // in hex characters.  2 hex digits plus a space
constexpr int DisplayCellsOfAddress = 4; // 16 bits (4 hex chars)
constexpr int DisplayCellsOfDecoratedAddress = 6; // '$' prefix.  ':' suffix
//constexpr int DisplayBytesPerLine   = 16;
//constexpr int LineNumberOfCells     = DisplayCellsOfAddress + 2 + DisplayBytesPerLine * DisplayCellsOfByte;
constexpr int LinesInDisplay        = 16;
constexpr int Spaces = 1;
constexpr int AddressPart = DisplayCellsOfDecoratedAddress + Spaces;
constexpr int DataPart = DisplayCellsOfByte * 16 + 15 * Spaces;

class PageView : public Node
{
public:
    PageView(std::shared_ptr<RamBusDeviceView> model,
             Ref<int>  current_byte,
             Ref<int>  program_counter,
             Ref<Decorator> edit_mode_decorator);

    void ComputeRequirement() override;

    void Render(Screen &screen) override;

protected:
    std::shared_ptr<RamBusDeviceView> _model;
    Ref<int>                          _current_byte_in_page;
    Ref<int>                          _program_counter;
    Ref<Decorator>                    _edit_mode_decorator;

    void drawLine(int line, Screen &screen);
    void drawAddress(int line, Screen &screen);
    void drawMemoryValues(int line, Screen &screen);
    int addressOfLine(int page, int linenumber);

    unsigned int memoryAt(int line, int column)
    {
        size_t page_index = _model->page() * 256;
        size_t line_index = page_index + line * 16;
        size_t index = line_index + static_cast<size_t>(column);

        return _model->model()->memory()[ index ];
    }
};

PageView::PageView(std::shared_ptr<RamBusDeviceView> model,
                   Ref<int>       current_byte,
                   Ref<int>       show_pc,
                   Ref<Decorator> edit_mode_decorator)
    :
    _model{ std::move(model) },
    _current_byte_in_page{ std::move(current_byte) },
    _program_counter{ std::move(show_pc) },
    _edit_mode_decorator{ std::move(edit_mode_decorator) }
{
}

void PageView::ComputeRequirement()
{
    requirement_.min_x = AddressPart + DataPart;
    requirement_.min_y = LinesInDisplay;
}

void PageView::Render(Screen &screen)
{
    for (int i = 0; i < LinesInDisplay; ++i)
        drawLine(i, screen);

    if (_current_byte_in_page() != -1)
    {
        auto cursor = pageview_ui_box_for_byte( _current_byte_in_page() );

        // Translate to global coords and draw...
        screen.PixelAt( box_.x_min + cursor.x_min    , box_.y_min + cursor.y_min ).inverted = true;
        screen.PixelAt( box_.x_min + cursor.x_min + 1, box_.y_min + cursor.y_min ).inverted = true;
    }

    if (_program_counter() != -1)
    {
        if ( (_program_counter() & 0xFF00) == (_model->page() << 8) )
        {
            // The program counter is in this page
            auto pc = pageview_ui_box_for_byte( _program_counter() & 0xFF );

            // Translate to global coords and draw...
            screen.PixelAt( box_.x_min + pc.x_min    , box_.y_min + pc.y_min ).background_color = Color::Blue;
            screen.PixelAt( box_.x_min + pc.x_min + 1, box_.y_min + pc.y_min ).background_color = Color::Blue;
        }
    }
}

void PageView::drawLine(int line, Screen &screen)
{
    drawAddress(line, screen);
    drawMemoryValues(line, screen);
}

void PageView::drawAddress(int line, Screen &screen)
{
    int address = addressOfLine(_model->page(), line);
    char buffer[AddressPart + 1];

    snprintf(buffer, sizeof(buffer), "$%.4X: ", static_cast<unsigned int>(address));

    for (int i = 0; i < AddressPart; ++i)
        screen.PixelAt(box_.x_min + i, box_.y_min + line).character = buffer[i];
}

void PageView::drawMemoryValues(int line, Screen &screen)
{
    char buffer[DataPart + 1];

    snprintf(buffer, sizeof(buffer), "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
             memoryAt(line, 0x0),
             memoryAt(line, 0x1),
             memoryAt(line, 0x2),
             memoryAt(line, 0x3),
             memoryAt(line, 0x4),
             memoryAt(line, 0x5),
             memoryAt(line, 0x6),
             memoryAt(line, 0x7),
             memoryAt(line, 0x8),
             memoryAt(line, 0x9),
             memoryAt(line, 0xA),
             memoryAt(line, 0xB),
             memoryAt(line, 0xC),
             memoryAt(line, 0xD),
             memoryAt(line, 0xE),
             memoryAt(line, 0xF)
             );

    for (int i = 0; i < DataPart; ++i)
        screen.PixelAt(box_.x_min + AddressPart + i, box_.y_min + line).character = buffer[i];
}

int PageView::addressOfLine(int page, int linenumber)
{
    return ((page << 8) + (16 * linenumber)) & 0xFFFF;
}

Element pageview(std::shared_ptr<RamBusDeviceView> model, Ref<int> current_byte, Ref<int> program_counter, Ref<Decorator> edit_mode_decorator)
{
    return std::make_shared<PageView>( std::move(model), std::move(current_byte), std::move(program_counter), std::move(edit_mode_decorator) );
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

            if (pos_in_byte < 2)
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
    int x_pos = AddressPart + line_pos * 3;

    return Box{ x_pos, x_pos + 1, line, line };
}
