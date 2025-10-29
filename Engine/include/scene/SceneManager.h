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

#include <string> 


class SceneManager{
public:
    struct Layer {
        std::vector<EntityID> entities{};
        LayerID id{ 0 }; //by default 0
        std::string name{};
    };
    struct Scene {
        std::vector<Layer> layers{};
        SceneID id{ 0 };//by default 0
        std::string name{};
    };

    void InitScene(SceneID);

    void Update();

    void Render();
    
    SceneID ResolveSceneKey(int idx) const; 

    void SetScenesJsonPath(const std::string & path);



private:
    std::unique_ptr<Scene> curr_scene{ nullptr };
    std::vector<Scene> scenes{};
};
