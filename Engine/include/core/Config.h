// load config (width/height/vsync) from file

/*!------------------------------------------------------------------------------------------------
    File: Config.h
    Project: NocturnalStudio

    This file contains 

    Primary Author:
        Nur Syazliyarni (100%)
        - 

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include <string>

#ifndef CONFIG_H
#define CONFIG_H

struct WindowConfig {
    int width;
    int height;
    bool vsync;
};

struct GameConfig {
    WindowConfig window;
};

GameConfig loadConfig(const std::string& filename);


#endif //CONFIG_H
