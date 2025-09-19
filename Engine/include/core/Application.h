// init(), run(), shutdown()

#pragma once
#include "graphics/Window.h"
#include "graphics/Texture.h"
#include "core/GameLoop.h"

class Application {
public:
    Application();
    int run();            // create window from JSON + minimal loop

private:
    Window m_window;
};
