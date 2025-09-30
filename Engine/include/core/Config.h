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