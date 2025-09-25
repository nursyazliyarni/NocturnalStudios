//<<<<<<< HEAD
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
#include <string>

rapidjson::Document WConfig;

GameConfig loadConfig(const std::string& filename) {
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("Could not open config file: " + filename);
    }

    char readBuffer[4096];
    rapidjson::FileReadStream is(file, readBuffer, sizeof(readBuffer));

    rapidjson::Document doc;
    doc.ParseStream(is);
    fclose(file);

    if (doc.HasParseError()) {
        throw std::runtime_error("JSON parse error in file: " + filename);
    }

    const auto& win = doc["window"];
    WindowConfig window{
        win["width"].GetInt(),
        win["height"].GetInt(),
        win["title"].GetString(),
        win["vsync"].GetBool()
    };

    return { window };
}

/*GameConfig loadConfig(const std::string& filename) {
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("Could not open config file : " + filename);

        char readBuffer[4096];

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
            win["title"].GetString(),
            win["vsync"].GetBool()
        };

        return { window };
    }
} */
// =======
// #include "core/Config.h"


// #include <rapidjson/document.h>
// #include <rapidjson/filereadstream.h>
// #pragma warning(pop)

// #include <cstdio>
// #include <vector>

// namespace Config {

//     WindowConfig LoadWindowConfig(const char* path) {
//         WindowConfig cfg; // start with defaults

//         if (FILE* fp = std::fopen(path, "rb")) {
//             std::vector<char> buf(1u << 16);
//             rapidjson::FileReadStream is(fp, buf.data(), buf.size());
//             rapidjson::Document doc; doc.ParseStream(is);
//             std::fclose(fp);

//             if (!doc.HasParseError() && doc.HasMember("window") && doc["window"].IsObject()) {
//                 const auto& w = doc["window"];
//                 if (w.HasMember("width") && w["width"].IsInt())    cfg.width = w["width"].GetInt();
//                 if (w.HasMember("height") && w["height"].IsInt())   cfg.height = w["height"].GetInt();
//                 if (w.HasMember("title") && w["title"].IsString()) cfg.title = w["title"].GetString();
//                 if (w.HasMember("vsync") && w["vsync"].IsBool())   cfg.vsync = w["vsync"].GetBool();
//             }
//         }
//         return cfg;
//     }

// } // namespace Config
// >>>>>>> Rishika
