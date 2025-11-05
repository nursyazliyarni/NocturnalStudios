#include "./scene/SceneManager.h"

#include "graphics/Camera2D.h" 
#include "scene/Prefab.h"       // Prefab_GetSceneName 


void SceneManager::InitScene(SceneID id) {
    for (const auto& s : scenes) {
        if (s.id == id) {
            /*curr_scene = std::make_unique<Scene>(s);
            return;*/

            curr_scene = std::make_unique<Scene>(s);

            //  camera preset activation 
            {
                const std::string& nm = Prefab_GetSceneName(id);
                if (!nm.empty()) Cam2D_ActivatePreset(nm);
            }

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


