#include "registerview.hpp"
#include "apputils.hpp"
#include <cstdlib>
#include <functional>
#include <array>
#include <charconv>

using namespace std;
using namespace ftxui;

const std::string RegisterView::EmptyRepresentation_Byte{"--"};
const std::string RegisterView::EmptyRepresentation_Word{"----"};

namespace
{

void ToHexString(uint8_t new_value, std::string &representation)
{
    char buffer[3];

    snprintf(buffer, sizeof(buffer), "%02X", new_value);
    representation.assign( buffer, sizeof(buffer) - 1 );
}

void ToHexString(uint16_t new_value, std::string &representation)
{
    char buffer[5];

    snprintf(buffer, sizeof(buffer), "%04X", new_value);
    representation.assign( buffer, sizeof(buffer) - 1 );
}

}
RegisterView::RegisterView(QObject *parent)
    :
    QObject(parent)
{
}

void RegisterView::setModel(olc6502 *new_model)
{
    if (new_model != _model)
    {
        if (_model)
            disconnectModelSignals(_model);
        _model = new_model;

        connectModelSignals(new_model);
        if (new_model)
        {
            // Let's go ahead and fill in the content to display...
            generateContent();
        }
        emit modelChanged();
    }
}

Component RegisterView::component()
{
    _inputs = Container::Vertical({
                                  _accumulator_input,
                                  _x_input,
                                  _y_input,
                                  _stack_pointer_input,
                                  _program_counter_input
    });
    return Renderer( CatchEvent( _inputs,
                                 std::bind( &RegisterView::onEvent, this, placeholders::_1 )
                               ),
                     std::bind( &RegisterView::generateView, this )
                   );
}

Element RegisterView::generateView() const
{
    return window( text("Registers") | hcenter,
                   hbox({
                       vbox({
                            text("A: "),
                            text("X: "),
                            text("Y: "),
                            text("SP: "),
                            text("PC: "),
                            text("ST: ")
                            }),
                      vbox({
                            hbox({ _accumulator_input->Render() | size(WIDTH, EQUAL, 2), filler() }),
                            hbox({ _x_input->Render() | size(WIDTH, EQUAL, 2), filler() }),
                            hbox({ _y_input->Render() | size(WIDTH, EQUAL, 2), filler() }),
                            hbox({ _stack_pointer_input->Render() | size(WIDTH, EQUAL, 2), filler() }),
                            hbox({ _program_counter_input->Render() | size(WIDTH, EQUAL, 4), filler() }),
                            hbox({ text("N") | color( statusBitState( FLAGS6502::N ) ), filler(),
                                   text("V") | color( statusBitState( FLAGS6502::V ) ), filler(),
                                   text("-"), filler(),
                                   text("B") | color( statusBitState( FLAGS6502::B ) ), filler(),
                                   text("D") | color( statusBitState( FLAGS6502::D ) ), filler(),
                                   text("I") | color( statusBitState( FLAGS6502::I ) ), filler(),
                                   text("Z") | color( statusBitState( FLAGS6502::Z ) ), filler(),
                                   text("C") | color( statusBitState( FLAGS6502::C ) )}) | size(WIDTH, EQUAL , 8 + 7)
                          })
                  }) ) |
           size(WIDTH, GREATER_THAN, strlen("Registers") + 1) | size(HEIGHT, EQUAL, 6 + 2);
}

ftxui::Color RegisterView::statusBitState(const FLAGS6502 bit) const
{
    return (_status_representation & bit) ? Color::Green : Color::Red;
}

void RegisterView::disconnectModelSignals(olc6502 *m)
{
    if (m)
    {
        m->disconnect(m,    &olc6502::aChanged,
                      this, &RegisterView::onAChanged);
        m->disconnect(m,    &olc6502::xChanged,
                      this, &RegisterView::onXChanged);
        m->disconnect(m,    &olc6502::yChanged,
                      this, &RegisterView::onYChanged);
        m->disconnect(m,    &olc6502::stackPointerChanged,
                      this, &RegisterView::onStackPointerChanged);
        m->disconnect(m,    &olc6502::pcChanged,
                      this, &RegisterView::onPCChanged);
        m->disconnect(m,    &olc6502::statusChanged,
                      this, &RegisterView::onStatusChanged);
    }
}

void RegisterView::connectModelSignals(olc6502 *m)
{
    if (m)
    {
        m->connect(m,    &olc6502::aChanged,
                   this, &RegisterView::onAChanged);
        m->connect(m,    &olc6502::xChanged,
                   this, &RegisterView::onXChanged);
        m->connect(m,    &olc6502::yChanged,
                   this, &RegisterView::onYChanged);
        m->connect(m,    &olc6502::stackPointerChanged,
                   this, &RegisterView::onStackPointerChanged);
        m->connect(m,    &olc6502::pcChanged,
                   this, &RegisterView::onPCChanged);
        m->connect(m,    &olc6502::statusChanged,
                   this, &RegisterView::onStatusChanged);
    }
}

void RegisterView::generateContent()
{
    onAChanged( model()->a() );
    onXChanged( model()->x() );
    onYChanged( model()->y() );
    onStackPointerChanged( model()->stackPointer() );
    onPCChanged( model()->pc() );
    onStatusChanged( model()->status() );
}

void RegisterView::onAChanged(uint8_t new_value)
{
    ToHexString( new_value, _a_representation );
}

void RegisterView::onXChanged(uint8_t new_value)
{
    ToHexString( new_value, _x_representation );
}

void RegisterView::onYChanged(uint8_t new_value)
{
    ToHexString( new_value, _y_representation );
}

void RegisterView::onStackPointerChanged(uint8_t new_value)
{
    ToHexString( new_value, _stack_pointer_representation );
}

void RegisterView::onPCChanged(uint16_t new_value)
{
    ToHexString( new_value, _program_counter_representation );
}

void RegisterView::onStatusChanged(uint8_t new_value)
{
    _status_representation = new_value;
}

bool RegisterView::onEvent(Event event)
{
    if ( !model() )
        return false;

    if ( _inputs->Focused() )
    {
        if ( editMode() )
        {
            if (event == Event::ArrowLeft)
            {
                {
                    auto child = _inputs->ActiveChild();

                    if ( child == _accumulator_input )
                        onAChanged( --_model->registers().a );
                    else if ( child == _x_input )
                        onXChanged( --_model->registers().x );
                    else if ( child == _y_input )
                        onYChanged( --_model->registers().y );
                    else if ( child == _stack_pointer_input )
                        onStackPointerChanged( --_model->registers().stack_pointer );
                    else if ( child == _program_counter_input )
                        onPCChanged( --_model->registers().program_counter );
                    else
                        return false;
                    return true;
                }
            }
            else if (event == Event::ArrowRight)
            {
                {
                    auto child = _inputs->ActiveChild();

                    if ( child == _accumulator_input )
                        onAChanged( ++_model->registers().a );
                    else if ( child == _x_input )
                        onXChanged( ++_model->registers().x );
                    else if ( child == _y_input )
                        onYChanged( ++_model->registers().y );
                    else if ( child == _stack_pointer_input )
                        onStackPointerChanged( ++_model->registers().stack_pointer );
                    else if ( child == _program_counter_input )
                        onPCChanged( ++_model->registers().program_counter );
                    else
                        return false;
                    return true;
                }
            }
        }

        if (event == Event::Return)
        {
            toggleEditMode();
            return true;
        }
    }

    return false;
}
