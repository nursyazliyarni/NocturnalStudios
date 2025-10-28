/*!------------------------------------------------------------------------------------------------
    File: LevelManager.h
    Project: NocturnalStudio

    This file contains ...

    Primary Author:
         Name (%)
        -Brief

    Secondary Author:
         Name (%)
        -Brief

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include <vector>
#include <stack>
#include <memory>
#include "../include/ECS/ECS.h"


class SceneManager {
public:

    struct Layer {
        std::vector<EntityID> entities{};
        LayerID id{ 0 }; //by default 0
        bool isActive{ false }, isVisible{ false };
        std::string name{};
    };
    struct Scene {
        std::vector<LayerID> layers{};
        LayerID layerCount{};
        SceneID id{ 0 };//by default 0, might use axewhys family thingy for this later
        std::string name{};
    };

private:
    std::shared_ptr<Scene> curr_scene{ nullptr };


    std::vector<LayerID> allLayers;

    std::unordered_map<SceneID, shared_ptr<Scene>> scene_prefabs;

    SceneManager(); //singleton, constructor is private.

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;

    void UnloadAllEntities();
    void LoadAllEntities();
public:
    static SceneManager& GetSceneManager() {
        static SceneManager thisInstance;
        return thisInstance;
    }

    void InitScene(SceneID);

    void Update();

    void Render();
};
