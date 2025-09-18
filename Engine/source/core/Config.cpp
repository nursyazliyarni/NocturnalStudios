// load config (width/height/vsync) from file

/*!------------------------------------------------------------------------------------------------
    File: Config.cpp
    Project: NocturnalStudio

    This file contains

    Primary Author:
        Nur Syazliyarni (100%)
        -

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/


#include "../include/core/Config.h"
//#include <iostream>
//#include <iomanip>
//#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <cstdio>
#include <stdexcept>

rapidjson::Document WConfig;

GameConfig loadConfig(const std::string& filename) {
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("Could not open config file : " + filename);

        char readBuffer[65536];

        rapidjson::FileReadStream is(file, readBuffer, sizeof(readBuffer));

        rapidjson::Document doc;
        doc.ParseStream(is);
        fclose(file);

        if (doc.HasParseError()) {
            throw std::runtime_error("JSON parse error in file: " + filename);
        }

        // Extract window config
        const auto& win = doc["window"];
        WindowConfig window{
            win["width"].GetInt(),
            win["height"].GetInt(),
            win["fullscreen"].GetBool()
        };

        return { window };
    }
}
