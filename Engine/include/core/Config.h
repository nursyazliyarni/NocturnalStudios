// load config (width/height/vsync) from file

#pragma once
#include <string>

//The defaults in Config.h are just a fallback if the JSON is missing/broken/not found.
struct WindowConfig {
    int         width = 1280;
    int         height = 720;
    std::string title = "NocturnalStudios";
    bool        vsync = true;
};

// Loads assets/config.json; if file missing/invalid, returns defaults above.
namespace Config {
    WindowConfig LoadWindowConfig(const char* path = "assets/config.json");
}
