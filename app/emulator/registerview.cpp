#include "registerview.hpp"
#include "apputils.hpp"
#include <cstdlib>
#include <functional>
#include <array>
#include <charconv>

using namespace ftxui;


RegisterView::RegisterView(QObject *parent)
    :
    QObject(parent)
{
    _input_a_option.on_change = [this]()
    {
        if ( model() )
        {
            _model->registers().a = *_input_a_option.data;
        }
    };

    _input_x_option.on_change = [this]()
    {
        if ( model() )
        {
            _model->registers().x = *_input_x_option.data;
        }
    };

    _input_y_option.on_change = [this]()
    {
        if ( model() )
        {
            _model->registers().y = *_input_y_option.data;
        }
    };

    _input_stack_pointer_option.on_change = [this]()
    {
        if ( model() )
        {
            _model->registers().stack_pointer = *_input_stack_pointer_option.data;
        }
    };

    _input_program_counter_option.on_change = [this]()
    {
        if ( model() )
        {
            _model->registers().program_counter = *_input_program_counter_option.data;
        }
    };
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
    return Renderer( _inputs, std::bind( &RegisterView::generateView, this ) );
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
                            hbox({ _accumulator_input->Render(), filler() }),
                            hbox({ _x_input->Render(), filler() }),
                            hbox({ _y_input->Render(), filler() }),
                            hbox({ _stack_pointer_input->Render(), filler() }),
                            hbox({ _program_counter_input->Render(), filler() }),
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
           size(WIDTH, EQUAL, 21) | size(HEIGHT, EQUAL, 6 + 2);
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
    *_input_a_option.data = new_value;
}

void RegisterView::onXChanged(uint8_t new_value)
{
    *_input_x_option.data = new_value;
}

void RegisterView::onYChanged(uint8_t new_value)
{
    *_input_y_option.data = new_value;
}

void RegisterView::onStackPointerChanged(uint8_t new_value)
{
    *_input_stack_pointer_option.data = new_value;
}

void RegisterView::onPCChanged(uint16_t new_value)
{
    *_input_program_counter_option.data = new_value;
}

void RegisterView::onStatusChanged(uint8_t new_value)
{
    _status_representation = new_value;
}
