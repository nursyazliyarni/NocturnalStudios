
/*!------------------------------------------------------------------------------------------------
    File: SceneManager.cpp
    Project: NocturnalStudio

    scene manager but niot un use yert

    Primary Author:
        Laycock Enhui Sarah (100%)
        - Created the skeleton of SceneManager

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "./scene/SceneManager.h"

#include "graphics/Camera2D.h" 
#include "scene/Prefab.h"       // Prefab_GetSceneName 

/**
 * A he;per function for M2 that a
 * 
 */
void SceneManagerTest() {

}

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


