#include "./scene/SceneManager.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

// [0]=ESC, [1..9]=digits
static std::array<SceneID, 10> sKey2Scene{};
static bool sKeymapInit = false;

// sceneName -> SceneID (built from JSON "scenes" key order aligned to in-memory scenes vector)
static std::unordered_map<std::string, SceneID> sNameToId;

// configurable JSON path (fallback stays this)
static std::string sScenesJsonPath = "assets/scenes/scenes.json";

static bool LoadScenesJson(rapidjson::Document& d) {
    std::ifstream f(sScenesJsonPath);
    if (!f) return false;
    rapidjson::IStreamWrapper isw(f);
    d.ParseStream(isw);
    return !d.HasParseError();
}

static SceneID NameToId(const char* nm) {
    if (!nm) return SceneID{};
    auto it = sNameToId.find(nm);
    return (it == sNameToId.end()) ? SceneID{} : it->second;
}


void SceneManager::InitScene(SceneID id) {
    for (const auto& s : scenes) {
        if (s.id == id) {
            curr_scene = std::make_unique<Scene>(s);
            return;
        }
    }
}

void SceneManager::Update() {
    // Build key?scene map once (rebuilt if SetScenesJsonPath is called)
    if (!sKeymapInit) {
        rapidjson::Document d;
        if (LoadScenesJson(d)) {
            // Build name?id WITHOUT referencing the 'Scene' type in a free function.
            sNameToId.clear();
            if (d.HasMember("scenes") && d["scenes"].IsObject()) {
                size_t idx = 0;
                for (auto it = d["scenes"].MemberBegin(); it != d["scenes"].MemberEnd(); ++it, ++idx) {
                    if (idx < scenes.size()) {
                        // scenes[idx] is your member vector; just read its id
                        sNameToId[it->name.GetString()] = scenes[idx].id;
                    }
                }
            }

            // states.initial / states.keymap
            if (d.HasMember("states") && d["states"].IsObject()) {
                const auto& st = d["states"];
                const char* initialName =
                    (st.HasMember("initial") && st["initial"].IsString()) ? st["initial"].GetString() : nullptr;

                if (st.HasMember("keymap") && st["keymap"].IsObject()) {
                    const auto& km = st["keymap"];

                    // digits 1..9
                    for (int i = 1; i <= 9; ++i) {
                        std::string key = std::to_string(i);
                        if (km.HasMember(key.c_str()) && km[key.c_str()].IsString()) {
                            sKey2Scene[static_cast<size_t>(i)] = NameToId(km[key.c_str()].GetString());
                        }
                    }

                    // ESC
                    if (km.HasMember("ESC") && km["ESC"].IsString())
                        sKey2Scene[0] = NameToId(km["ESC"].GetString());
                    else if (initialName)
                        sKey2Scene[0] = NameToId(initialName);
                }
                else if (initialName) {
                    // no keymap ? still map ESC to initial
                    sKey2Scene[0] = NameToId(initialName);
                }
            }
        }

        // Fallback: nothing mapped ? 1..N from in-memory order, ESC?first
        bool any = false;
        for (int i = 0; i < 10; ++i) {
            if (sKey2Scene[static_cast<size_t>(i)] != SceneID{}) {
                any = true;
                break;
            }
        }
        if (!any && !scenes.empty()) {
            const size_t n = scenes.size();
            for (size_t i = 1; i <= 9 && i <= n; ++i)
                sKey2Scene[i] = scenes[i - 1].id;
            sKey2Scene[0] = scenes.front().id;
        }

        sKeymapInit = true;
    }

    if (curr_scene) {
        for (const auto& l : curr_scene->layers) {
            (void)l; // TODO: logic/physics per layer
        }
    }
}

void SceneManager::Render() {
    if (curr_scene) {
        for (const auto& l : curr_scene->layers) {
            (void)l; // TODO: render per layer
        }
    }
}

SceneID SceneManager::ResolveSceneKey(int idx) const {
    if (idx < 0 || idx > 9) return SceneID{};
    return sKey2Scene[static_cast<size_t>(idx)];
}

void SceneManager::SetScenesJsonPath(const std::string& p) {
    sScenesJsonPath = p;
    sKeymapInit = false; // force rebuild next Update()
}
