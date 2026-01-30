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
    GameManager::GameManager() :gameTimeElapsed{ 0.f },currScene{GameScene::NONE}
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
                    
                }
            }
            gameTimeElapsed += dt;
        }


        
    }

    void GameManager::LoadPersistentValues() {

    }
    /*!*************************************************************************
    This function ends the system. As there are no data members that require
    initialization, there are no data members that need to be un-initialised.
    Therefore this function is empty
    ****************************************************************************/
    void GameManager::Restart() {}
}