/*!------------------------------------------------------------------------------------------------
    File: LevelManager.h
    Project: NocturnalStudio

    This file contains ...

     Author:
         Sarah (%)
        -Brief


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include <vector>
#include <stack>
#include <memory>
#include "../include/ECS/ECS.h"

#include "scene/Prefab.h"  
#include <string> 



class SceneManager{
public:
    struct Layer {
        std::vector<EntityID> entities{};
        LayerID l_id{ 0 }; //by default 0
        std::string l_name{};
    };
    struct Scene {
        std::vector<Layer> layers{};
        SceneID s_id{ 0 };//by default 0
        std::string name{};
    };

    void InitScene(SceneID);

    void Update();

    void Render();
    
    void SceneManagerTest(PrefabManager& p);

    static SceneManager& GetSceneManager() {
        static SceneManager dis;
        return dis;
    }

private:
    SceneManager(); //singleton, constructor is private.
    
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;

    std::unique_ptr<Scene> curr_scene{ nullptr };
    std::vector<Scene> scenes{};
};
