QT -= gui

CONFIG += c++17
CONFIG += console
CONFIG -= app_bundle

APPDIR = $$PWD/../app

INCLUDEPATH += $$APPDIR

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        $$APPDIR/io/SRecord/QSRecordStream.cpp \
        $$APPDIR/io/SRecord/srecord.cpp \
        $$APPDIR/io/SimpleHex/QSimpleHexStream.cpp \
        test_simplehex.cpp \
        test_srecord.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
        $$APPDIR/io/memory_block.hpp \
        $$APPDIR/io/SRecord/srecord.hpp \
        $$APPDIR/io/SRecord/QSRecordStream.hpp \
        $$APPDIR/io/SimpleHex/QSimpleHexStream.hpp \
        test_simplehex.hpp \
        test_srecord.hpp


