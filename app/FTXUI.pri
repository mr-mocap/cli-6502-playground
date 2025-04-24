FTXUIDIR = $$PWD/../FTXUI

INCLUDEPATH += $$FTXUIDIR/include

# You will need to change this to point to your own build for now.
# Please note that this was built on Windows, so some adjustment will be
# necessary for other platforms.
unix: LIBS += -L$$FTXUIDIR/build
win32: CONFIG(Release) {
LIBS += -L$$FTXUIDIR/build/Release
}
win32: CONFIG(Debug) {
LIBS += -L$$FTXUIDIR/build/Debug
}
win32: CONFIG(RelWithDebInfo) {
LIBS += -L$$FTXUIDIR/build/RelWithDebInfo
}

LIBS += -lftxui-component -lftxui-dom -lftxui-screen
