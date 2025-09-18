#include "core/Config.h"


#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#pragma warning(pop)

#include <cstdio>
#include <vector>

namespace Config {

    WindowConfig LoadWindowConfig(const char* path) {
        WindowConfig cfg; // start with defaults

        if (FILE* fp = std::fopen(path, "rb")) {
            std::vector<char> buf(1u << 16);
            rapidjson::FileReadStream is(fp, buf.data(), buf.size());
            rapidjson::Document doc; doc.ParseStream(is);
            std::fclose(fp);

            if (!doc.HasParseError() && doc.HasMember("window") && doc["window"].IsObject()) {
                const auto& w = doc["window"];
                if (w.HasMember("width") && w["width"].IsInt())    cfg.width = w["width"].GetInt();
                if (w.HasMember("height") && w["height"].IsInt())   cfg.height = w["height"].GetInt();
                if (w.HasMember("title") && w["title"].IsString()) cfg.title = w["title"].GetString();
                if (w.HasMember("vsync") && w["vsync"].IsBool())   cfg.vsync = w["vsync"].GetBool();
            }
        }
        return cfg;
    }

} // namespace Config
