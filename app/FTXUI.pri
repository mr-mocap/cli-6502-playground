FTXUIDIR = $$PWD/../FTXUI

INCLUDEPATH += $$FTXUIDIR/include

# You will need to change this to point to your own build for now.
# Please note that this was build on Windows, so some adjustment will be
# necessary for other platforms.
unix: LIBS += -L$$FTXUIDIR/build -lftxui-component -lftxui-dom -lftxui-screen
win32: LIBS += -L$$FTXUIDIR/build/RelWithDebInfo -lftxui-component -lftxui-dom -lftxui-screen

message($$FTXUIDIR)
message($$LIBS)
