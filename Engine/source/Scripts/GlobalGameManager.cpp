/*!------------------------------------------------------------------------------------------------
    File: GameManager.cpp
    Project: NocturnalStudio

    Brief:
        This file contains the implementation of the GameManager class which is responsible
        for handling collision detection and response within the physics engine.

    Primary Author:
         LAU XIANG YANG (100%)
        - Created the GameManager class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/GlobalGameManager.h"
#include "ECS/ECS.h"
#include "GUI/GUI.h"

namespace NocturnalEngine {

    GameManager GameManager::instance;

    /*!*************************************************************************
    Returns an instance of the MemoryManager
    ****************************************************************************/
    GameManager& GameManager::GetInstance()
    {
        return instance;
    }
    /*!*************************************************************************
    Constructor
    ****************************************************************************/
    GameManager::GameManager() :gameTimeElapsed{ 0.f }, currScene{ GameScene::NONE }, sceneAnimalMap{}
    {


    }

    /*!*************************************************************************
    Destructor
    ****************************************************************************/
    GameManager::~GameManager()
    {
        
    }
    /*!*************************************************************************
    This function initialises the system. As there are no data members that require
    initialization, this function is empty
    ****************************************************************************/
    void GameManager::Init() {
        //this is default for the sake of the game working, 
        // when serialization is fixed, It will be able to detect 
        AnimalAttributes defaultAttributes{};
        defaultAttributes.SetDefault();

        sceneAnimalMap.emplace(GameScene::FLAMINGO, defaultAttributes);
        sceneAnimalMap.emplace(GameScene::HORSE, defaultAttributes);
        sceneAnimalMap.emplace(GameScene::OWL, defaultAttributes);
        sceneAnimalMap.emplace(GameScene::SEAL, defaultAttributes);
    }
    /*!*************************************************************************
    This function runs the logic of the system. It checks the list of entities with
    a collision component in pairs to see which of them are colliding. If two objects
    are colliding, collision response is dictated by the type of collider the object
    has
    ****************************************************************************/
    void GameManager::Update(float dt) {
       
        if (!p_GUI->check_pause())
        {
            for (std::pair<GameScene,AnimalAttributes> sceneAnimalPair:sceneAnimalMap)
            {
                //this processes the other animals in the 
                // background to give the illusion of persistent data
                //For the animal currently in scene, refer to Scripts/AnimalLogic.cpp
                // The code should be the exact same
                if (sceneAnimalPair.first!=currScene)
                {
                    sceneAnimalPair.second.current_hunger -= sceneAnimalPair.second.hungerDecayRate * dt * 50;
                    sceneAnimalPair.second.current_health -= sceneAnimalPair.second.healthDecayRate * dt;
                    sceneAnimalPair.second.current_sanity -= sceneAnimalPair.second.sanityDecayRate * dt;


                    if (sceneAnimalPair.second.current_hunger <= 0 || sceneAnimalPair.second.current_health <= 0 || sceneAnimalPair.second.current_sanity <= 0) {
                        sceneAnimalPair.second.is_alive = false;
                    }
                
                }
            }
            gameTimeElapsed += dt;
        }
    }

        
    

    AnimalAttributes GameManager::LoadPersistentValues(GameScene gs) {
        auto it = sceneAnimalMap.find(gs);
        if (it != sceneAnimalMap.end()) {
            currScene = gs;
            return it->second;
        }
        else {
            AnimalAttributes defaultAttributes{};
            defaultAttributes.SetDefault();
            return defaultAttributes;
        }
    }
    /*!*************************************************************************
    This function ends the system. As there are no data members that require
    initialization, there are no data members that need to be un-initialised.
    Therefore this function is empty
    ****************************************************************************/
    void GameManager::Restart() {}

    void GameManager::SaveAnimalAttributesToCurrentScene(AnimalAttributes aA) {
        auto it = sceneAnimalMap.find(currScene);

        //there is an older version of the aa in the thingy
        //oh i have to rekmember to iterate this with one by default ops
        if (it == sceneAnimalMap.end()) {
            sceneAnimalMap.emplace(currScene, aA);
        }
        else {
            it->second = aA;
        }
    }
}