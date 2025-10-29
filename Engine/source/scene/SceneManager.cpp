#include "./scene/SceneManager.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>






void SceneManager::InitScene(SceneID id) {
    for (const auto& s : scenes) {
        if (s.id == id) {
            curr_scene = std::make_unique<Scene>(s);
            return;
        }
    }
}

void SceneManager::Update() {
    

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


