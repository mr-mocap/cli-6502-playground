FTXUIDIR = $$PWD/../FTXUI

INCLUDEPATH += $$FTXUIDIR/include

# You will need to change this to point to your own build for now.
# Please note that this was build on Windows, so some adjustment will be
# necessary for other platforms.
unix: LIBS += -L$$FTXUIDIR/build -lftxui-component -lftxui-dom -lftxui-screen
win32: CONFIG(Release) {
LIBS += -L$$FTXUIDIR/build/Release -lftxui-component -lftxui-dom -lftxui-screen
}
win32: CONFIG(Debug) {
LIBS += -L$$FTXUIDIR/build/Debug -lftxui-component -lftxui-dom -lftxui-screen
}
