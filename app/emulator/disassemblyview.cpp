#include "disassemblyview.hpp"

using namespace ftxui;

class DisassemblyView : public Node
{
public:
    DisassemblyView(RamBusDevice *ram, const olc6502 *model) : _ram(ram), _model(model) { }

    void ComputeRequirement() override;

    void Render(Screen &screen) override;

protected:
    RamBusDevice  *_ram;
    const olc6502 *_model;
};
