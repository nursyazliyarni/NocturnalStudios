#include "./scene/SceneManager.h"

#include "graphics/Camera2D.h" 
#include "scene/Prefab.h"       // Prefab_GetSceneName 
namespace {
    int numbox{ 3 };
    ECS& ecs = ECS::getECS();
}
/**
 * A he;per function for M2 that a
 * 
 */
void SceneManager::SceneManagerTest(PrefabManager& p) {
    
    if (curr_scene->layers[0].entities.size()<numbox)
    {
        
        curr_scene->layers[0].entities.push_back(p.Instantiate("toy", "kms"));
    }
}

void SceneManager::InitScene(SceneID id) {
    for (const auto& s : scenes) {
        if (s.s_id == id) {
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

SceneManager::SceneManager() {
    Scene d{ {},0,"Testscn" };
    d.layers.push_back({ {},1,"TEST" });
    scenes.push_back(d);

    InitScene(0);

    curr_scene->layers[0].entities.push_back(0);
}
