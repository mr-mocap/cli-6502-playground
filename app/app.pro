QT -= gui

CONFIG += c++17
CONFIG += console
CONFIG -= app_bundle

APPDIR = $$PWD

INCLUDEPATH += $$APPDIR

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        apputils.cpp \
        cliplaygroundapplication.cpp \
        utilities/StringConversions.cpp \
        emulator/bus.cpp \
        emulator/computer.cpp \
        emulator/disassembly.cpp \
        emulator/disassemblyview.cpp \
        emulator/ibusdevice.cpp \
        emulator/instructionexecutor.cpp \
        emulator/memorypage.cpp \
        emulator/olc6502.cpp \
        emulator/pageview.cpp \
        emulator/rambusdevice.cpp \
        emulator/rambusdeviceview.cpp \
        emulator/registerview.cpp \
        ui/components/directorybrowser.cpp \
        ui/components/inputnumber.cpp \
        ui/components/list.cpp \
	io/SRecord/srecord.cpp \
	io/SRecord/QSRecordStream.cpp \
        io/SimpleHex/QSimpleHexStream.cpp \
        io/io.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    apputils.hpp \
    cliplaygroundapplication.h \
    utilities/StringConversions.hpp \
    emulator/bus.hpp \
    emulator/computer.hpp \
    emulator/disassembly.hpp \
    emulator/disassemblyview.hpp \
    emulator/flags.hpp \
    emulator/ibusdevice.hpp \
    emulator/instructionexecutor.hpp \
    emulator/memorypage.hpp \
    emulator/olc6502.hpp \
    emulator/pageview.hpp \
    emulator/rambusdevice.hpp \
    emulator/rambusdeviceview.hpp \
    emulator/registers.hpp \
    emulator/registerview.hpp \
    ui/components/directorybrowser.hpp \
    ui/components/inputnumber.hpp \
    ui/components/list.hpp \
    io/memory_block.hpp \
    io/SRecord/srecord.hpp \
    io/SRecord/QSRecordStream.hpp \
    io/SimpleHex/QSimpleHexStream.hpp \
    io/io.hpp

include(FTXUI.pri)
