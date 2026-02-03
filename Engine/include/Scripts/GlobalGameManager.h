/*!------------------------------------------------------------------------------------------------
    File: GameManager.h
    Project: NocturnalStudio

    Brief:
        This file contains the system declaration for the collision system to
        detect collision between two entities and calculate the appropriate physics
        response.

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the GameManager.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "ECS/Components/Components.h"
#include "pch.h"
#include "System/System.h"

#define p_GameManager NocturnalEngine::GameManager::GetInstance()

namespace NocturnalEngine {
    class GameManager {
    public:
        enum class GameScene {
            NONE = 0,
            FLAMINGO,
            HORSE,
            SEAL,
            OWL,
            STORAGE
        };

        enum class GameState {
            STARTING,
            PLAYING,
            ENDING
        };
        AnimalAttributes LoadPersistentValues(GameScene);

        static GameManager& GetInstance();
        /*!*************************************************************************
        This function initialises the system. As there are no data members that require
        initialization, this function is empty
        ****************************************************************************/
         void Init() ;
        /*!*************************************************************************
        This function runs the logic of the system. It checks the list of entities with
        a collision component in pairs to see which of them are colliding. If two objects
        are colliding, collision response is dictated by the type of collider the object
        has
        ****************************************************************************/
         void Update(float dt) ;
        /*!*************************************************************************
        This function ends the system. As there are no data members that require
        initialization, there are no data members that need to be un-initialised.
        Therefore this function is empty
        ****************************************************************************/
         void ResetSystem(bool editorMode = false) ;

         void SaveAnimalAttributesToCurrentScene(AnimalAttributes aA);

         GameState GetGameState()const { return currState; }

         void SetState(GameState s) { currState = s; }
        
         void AddDeath() { deadAnimals++; }

         //returns how much we have to multiply seconds by for timer
         float GetTimeScale() const{ return 21600.f/winRequirement; }

         float GetTimeElapsed() const{ return gameTimeElapsed; }

         bool HasLogBeenUpdated();

         const std::deque<std::string>& GetLogEntries()  { return diegeticLogEntries; }

         void AddLogEntry(std::string message);

         bool isTargetVisible()const { return targetVisibility; }

         void setTargetVisibility(bool t) { targetVisibility = t; }

         bool IsNewSceneManagement() const { return freakyMode; }
    private:

        GameManager();
        ~GameManager();
        GameManager(const GameManager&) = delete;
        GameManager& operator=(const GameManager&) = delete;

        static GameManager instance;

        void LoseGame();
        void WinGame();
        void LogLowHealth(std::pair<const GameScene, AnimalAttributes>& sceneAnimalPair);

        float gameTimeElapsed{}, winRequirement{180.f}; //in seconds

        GameScene currScene;
        GameState currState;
        std::unordered_map < GameScene, AnimalAttributes> sceneAnimalMap;
        int deadAnimals, deathLimit{ 2 }, maxDiegeticLogEntries{40};
        std::deque<std::string> diegeticLogEntries;

        bool diegeticLogUpdated{ true }, targetVisibility{ false }, freakyMode{ false };

        void CombinedSceneLogic(float dt);

    };


}