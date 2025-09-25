// load config (width/height/vsync) from file

//<<<<<<< HEAD
/*!------------------------------------------------------------------------------------------------
    File: Config.h
    Project: NocturnalStudio

    This file contains 

    Primary Author:
        Nur Syazliyarni (100%)
        - 

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
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


#endif //CONFIG_H
// =======
// #pragma once
// #include <string>

// //The defaults in Config.h are just a fallback if the JSON is missing/broken/not found.
// struct WindowConfig {
//     int         width = 1280;
//     int         height = 720;
//     std::string title = "NocturnalStudios";
//     bool        vsync = true;
// };

// // Loads assets/config.json; if file missing/invalid, returns defaults above.
// namespace Config {
//     WindowConfig LoadWindowConfig(const char* path = "assets/config.json");
// }
// >>>>>>> Rishika
