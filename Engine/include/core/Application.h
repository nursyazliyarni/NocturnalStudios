// init(), run(), shutdown()

#pragma once
#include "graphics/Window.h"

class Application {
public:
    Application();
    int run();            // create window from JSON + minimal loop

private:
    Window m_window;
};
