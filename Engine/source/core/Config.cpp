#include "core/Config.h"


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
