/*!------------------------------------------------------------------------------------------------
    File: Config.h
    Project: NocturnalStudio

    This file contains ...

    Primary Author:
         Name (%)
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
    int scene_id{ 0 };
};

GameConfig loadConfig(const std::string& filename);


#endif 

