/*!------------------------------------------------------------------------------------------------
    File: Config.h
    Project: NocturnalStudio

    This file contains the windowconfig struct and declaration
    of function loadconfig

    Primary Author:
         Nur Syazliyarni (100%)
        -Brief

    Secondary Author:
         Name (%)
        -Brief

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// load config (width/height/vsync) from file

#pragma once
#include <string>

#ifndef CONFIG_H
#define CONFIG_H

struct WindowConfig {
    int width;
    int height;
    std::string title = "NocturnalStudios";
    bool vsync;
};

struct GameConfig {
    WindowConfig window;
};

GameConfig loadConfig(const std::string& filename);


#endif 