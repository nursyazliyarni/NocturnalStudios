#include "./scene/SceneManager.h"

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


