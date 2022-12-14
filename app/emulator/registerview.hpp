#ifndef REGISTERVIEW_HPP
#define REGISTERVIEW_HPP

#include <QObject>
#include "ftxui/component/component.hpp"          // for Make, Input
#include "emulator/inputnumber.hpp"
#include "emulator/olc6502.hpp"

class RegisterView : public QObject
{
    Q_OBJECT
public:
    explicit RegisterView(QObject *parent = nullptr);
    ~RegisterView() = default;

    /** Retrieve the underlying model.
     *
     *  @return A pointer to the underlying model
     */
    ///@{
    const olc6502 *model() const { return _model; }
          olc6502 *model()       { return _model; }
    ///@}

    /** Sets the underlying model of this view.
     *
     *  @param new_model The model to view
     */
    void setModel(olc6502 *new_model);

    bool editMode() const { return _edit_mode; }
    void toggleEditMode() { _edit_mode = !_edit_mode; }

    ftxui::Component component();
signals:
    /** Emitted when the underlying model is set or reset.
     *
     *  @see model
     *  @see setModel
     */
    void modelChanged();

protected:
    olc6502          *_model = nullptr;
    InputByteOption   _input_a_option;
    InputByteOption   _input_x_option;
    InputByteOption   _input_y_option;
    InputByteOption   _input_stack_pointer_option;
    InputWordOption   _input_program_counter_option;
    uint8_t           _a_representation_byte = 0;
    uint8_t           _x_representation_byte = 0;
    uint8_t           _y_representation_byte = 0;
    uint8_t           _stack_pointer_representation_byte = 0;
    uint16_t          _program_counter_representation_byte = 0;
    uint8_t           _status_representation;
    ftxui::Component  _accumulator_input = InputByte(&_a_representation_byte, &_input_a_option);
    ftxui::Component  _x_input = InputByte(&_x_representation_byte, &_input_x_option);
    ftxui::Component  _y_input = InputByte(&_y_representation_byte, &_input_y_option);
    ftxui::Component  _stack_pointer_input = InputByte(&_stack_pointer_representation_byte, &_input_stack_pointer_option);
    ftxui::Component  _program_counter_input = InputWord(&_program_counter_representation_byte, &_input_program_counter_option);
    ftxui::Component  _inputs;
    bool              _edit_mode = false;

    void disconnectModelSignals(olc6502 *m);
    void connectModelSignals(olc6502 *m);
    void generateContent();
    ftxui::Element generateView() const;
    ftxui::Color statusBitState(const FLAGS6502 flag) const;

private slots:
    void onAChanged(uint8_t new_value);
    void onXChanged(uint8_t new_value);
    void onYChanged(uint8_t new_value);
    void onPCChanged(uint16_t new_value);
    void onStackPointerChanged(uint8_t new_value);
    void onStatusChanged(uint8_t new_value);
};

#endif // REGISTERVIEW_HPP
