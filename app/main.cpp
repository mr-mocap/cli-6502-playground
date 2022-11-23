#include "cliplaygroundapplication.h"

int main(int argc, char *argv[])
{
    CLIPlaygroundApplication app(argc, argv);

    app.setup_ui();

    return app.mainLoop();
}
