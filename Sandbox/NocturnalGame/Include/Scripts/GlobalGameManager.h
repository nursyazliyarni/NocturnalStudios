/*!------------------------------------------------------------------------------------------------
    File: GlobalGameManager.h
    Project: NocturnalStudio

    Brief:
        This file contains the Game manager which is responsible for handling logic
        of animals that are currently not being diaplayed in the current scene but
        are still in the game

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the GameManager.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "ECS/Components/Components.h"
#include "pch.h"
#include "System/System.h"
#include "ECS/Components/AnimalAttributes.h"

#define p_GameManager NocturnalEngine::GameManager::GetInstance()

namespace NocturnalEngine {
    class GameManager {
    public:

        enum class GameState {
            STARTING,
            PLAYING,
            ENDING
        };
        AnimalAttributes LoadPersistentValues(std::string gs);

        static GameManager& GetInstance();
        /*!*************************************************************************
        This function initialises the system. 
        ****************************************************************************/
         void Init() ;
        /*!*************************************************************************
        This function runs the logic of the gamemanager. It handles the animals that 
        are not in the current scene and the logic that goes witht hem
        ****************************************************************************/
         void Update(float dt) ;
        /*!*************************************************************************
        This function ends the system. 
        ****************************************************************************/
         void ResetSystem(bool editorMode = false) ;

         void ResetStats() {
             deadAnimals = 0;
             escapedAnimals = 0;
             under20 = 0;
         }

         void SaveAnimalAttributesToCurrentScene(AnimalAttributes aA);

         GameState GetGameState()const { return currState; }

         void SetState(GameState s) { currState = s; }
        
         void AddDeath() { deadAnimals++; }

         void AddEscape() { escapedAnimals++; }

         void AddCrit() { under20++; }

         int AnimalsGone() const { return deadAnimals + escapedAnimals; }

         int getCrits() const{ return under20; }

         //returns how much we have to multiply seconds by for timer
         float GetTimeScale() const{ return 21600.f/winRequirement; }

         float GetTimeElapsed() const{ return gameTimeElapsed; }

         bool HasLogBeenUpdated();

         const std::deque<std::string>& GetLogEntries()  { return diegeticLogEntries; }

         void AddLogEntry(std::string message);

         bool isTargetVisible()const { return targetVisibility; }

         void setTargetVisibility(bool t) { targetVisibility = t; }

         bool IsNewSceneManagement() const { return altMode; }

         void ToggleSceneManagement() { altMode = !altMode; }

         void SetSceneToSwitch(std::string);
    private:

        GameManager();
        ~GameManager();
        GameManager(const GameManager&) = delete;
        GameManager& operator=(const GameManager&) = delete;

        static GameManager instance;

        void LoseGame();
        void WinGame();
        void LogLowHealth(std::pair < const std ::string , AnimalAttributes > & sceneAnimalPair);


        float gameTimeElapsed{}, winRequirement{120.f}; //in seconds

        std::string currScene;
        GameState currState;
        std::unordered_map < std::string, AnimalAttributes> sceneAnimalMap;
        int deadAnimals, deathLimit{ 3 }, maxDiegeticLogEntries{ 40 }, escapedAnimals{ 0 }, under20{ 0 };
        std::deque<std::string> diegeticLogEntries;

        bool diegeticLogUpdated{ true }, targetVisibility{ false };


        //all these is for another method of loading scenes i am proposing
        bool altMode{ false }, combinedSceneSetup{ false };
        int innerState{ 0 };
        Entity animalE{}, cageBGE{}, containmentE[3]{0,0,0};
        struct AnimalSceneData
        {
            Transform aTransform;
            Sprite aniSprite,bgSprite;
            Collider aCollider;
            int bgAudioInd;
            std::string containmentEntries[3];
        };
        std::string nextScene{ "Flamingo" };
        std::unordered_map < std::string, AnimalSceneData> combinedSceneMap;
        void CombinedSceneStart();
        void CombinedSceneUpdate(float dt);
        Entity GetEntityByName(const std::string name);


    };


}