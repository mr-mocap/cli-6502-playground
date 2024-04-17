#ifndef COMPUTER_HPP
#define COMPUTER_HPP

#include <QObject>
#include <QTimer>
#include "olc6502.hpp"
#include "bus.hpp"
#include "rambusdevice.hpp"
#include "io/io.hpp"


class Computer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(olc6502      *cpu READ cpu CONSTANT FINAL)
    Q_PROPERTY(RamBusDevice *ram READ ram CONSTANT FINAL)
public:
    explicit Computer(QObject *parent = nullptr);

    void loadProgram(QString path);

public slots:
    void startClock();
    void stopClock();
    void stepClock();

    void stepInstruction(int number_of_instructions = 1);

    const olc6502 *cpu() const { return &_cpu; }
          olc6502 *cpu()       { return &_cpu; }

    const RamBusDevice *ram() const { return &_memory; }
          RamBusDevice *ram()       { return &_memory; }

signals:

private slots:
    void timerTimeout();

private:
    olc6502 _cpu;
    Bus     _bus;
    RamBusDevice _memory;
    QTimer       _clock;

    void load(const MemoryBlock &mb);

    Q_DISABLE_COPY(Computer)
};

#endif // COMPUTER_HPP
