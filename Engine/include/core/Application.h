/*!------------------------------------------------------------------------------------------------
    File: Application.h
    Project: NocturnalStudio

	This file contains Application class to start the application defined in Application.cpp.

    Rishika Bimal Attodi
        - Declared Application interface and main run entrypoint.

    Secondary Author:
         Name (%)
        -Brief

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// init(), run(), shutdown()

#pragma once
#include "graphics/Window.h"
#include "graphics/Texture.h"
#include "core/GameLoop.h"
#include "graphics/Shader.h"
#include "graphics/Renderer.h"
#include "physics/Physics.h"
#include "core/Logger.h"

class Application {
public:
    Application();
    int run();            // create window from JSON + minimal loop

private:
    Window m_window;
};
