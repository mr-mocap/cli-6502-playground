#ifndef REGISTERVIEW_HPP
#define REGISTERVIEW_HPP

#include <QObject>
#include "ftxui/component/component.hpp"          // for Make, Input
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
    std::string       _a_representation;
    std::string       _x_representation;
    std::string       _y_representation;
    std::string       _stack_pointer_representation;
    std::string       _program_counter_representation;
    uint8_t           _status_representation;
    ftxui::Component  _accumulator_input = ftxui::Input(&_a_representation, EmptyRepresentation_Byte);
    ftxui::Component  _x_input = ftxui::Input(&_x_representation, EmptyRepresentation_Byte);
    ftxui::Component  _y_input = ftxui::Input(&_y_representation, EmptyRepresentation_Byte);
    ftxui::Component  _stack_pointer_input = ftxui::Input(&_stack_pointer_representation, EmptyRepresentation_Byte);
    ftxui::Component  _program_counter_input = ftxui::Input(&_program_counter_representation, EmptyRepresentation_Word);
    ftxui::Component  _inputs;
    bool              _edit_mode = false;

    static const std::string EmptyRepresentation_Byte;
    static const std::string EmptyRepresentation_Word;

    void disconnectModelSignals(olc6502 *m);
    void connectModelSignals(olc6502 *m);
    void generateContent();
    ftxui::Element generateView() const;
    bool onEvent(ftxui::Event event);
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
