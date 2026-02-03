/*!------------------------------------------------------------------------------------------------
    File: GameManager.cpp
    Project: NocturnalStudio

    Brief:
        This file contains the implementation of the GameManager class which is responsible
        for handling collision detection and response within the physics engine.

    Primary Author:
         LAUYCOCK ENHUI SARAH (100%)
        - Created the GameManager class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/GlobalGameManager.h"
#include "ECS/ECS.h"
#include "GUI/GUI.h"
#include "ECS/SceneManager.h"

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
        sceneAnimalMap[GameScene::FLAMINGO].animalType = AnimalAttributes::AnimalType::FLAMINGO;
        sceneAnimalMap.emplace(GameScene::HORSE, defaultAttributes);
        sceneAnimalMap[GameScene::HORSE].animalType = AnimalAttributes::AnimalType::HOESE;
        sceneAnimalMap.emplace(GameScene::OWL, defaultAttributes);
        sceneAnimalMap[GameScene::OWL].animalType = AnimalAttributes::AnimalType::OWL;
        sceneAnimalMap[GameScene::OWL].hungerDecayRate = 1.5; //owl special hunger
        sceneAnimalMap.emplace(GameScene::SEAL, defaultAttributes);
        sceneAnimalMap[GameScene::SEAL].animalType = AnimalAttributes::AnimalType::SEAL;
        currState=  GameManager::GameState::PLAYING;
        deadAnimals = 0;
    }

    void GameManager::CombinedSceneStart() {
        
        
    }
    /*!*************************************************************************
    This function runs the logic of the system. It checks the list of entities with
    a collision component in pairs to see which of them are colliding. If two objects
    are colliding, collision response is dictated by the type of collider the object
    has
    ****************************************************************************/
    void GameManager::Update(float dt) {
       
        if (IsNewSceneManagement() && !combinedSceneSetup) CombinedSceneStart();

        if (p_Scene->isGameplay)
        {
            if (IsNewSceneManagement())CombinedSceneUpdate(dt);
            for (std::pair<const GameScene,AnimalAttributes>& sceneAnimalPair:sceneAnimalMap)
            {
                //this processes the other animals in the 
                // background to give the illusion of persistent data
                //For the animal currently in scene, refer to Scripts/AnimalLogic.cpp
                // The code should be the exact same
                if (sceneAnimalPair.first!=currScene&& sceneAnimalPair.second.is_alive)
                {
                    sceneAnimalPair.second.current_hunger -= sceneAnimalPair.second.hungerDecayRate * static_cast<double>( dt) * 10.0;
                    // Adjust health decay based on animal type and hunger state
                    const double baseHealthMultiplier = 3.0; // much faster overall
                    double healthMultiplier = baseHealthMultiplier;

                    const double hungerRatio = (sceneAnimalPair.second.maxHunger > 0.0) ? (sceneAnimalPair.second.current_hunger / sceneAnimalPair.second.maxHunger) : 0.0;
                    const double lowHungerThreshold = 0.30;   // below 30% is "low"
                    const double highHungerThreshold = 0.70;  // above 70% is "normal+"

                    if (sceneAnimalPair.second.WhatKindOfAnimalIsThis() == "Flamingo")
                    {
                        if (hungerRatio < lowHungerThreshold)
                            healthMultiplier = 2.0; // faster when hungry
                        else if (hungerRatio > highHungerThreshold)
                            healthMultiplier = 0.5; // slower when well fed
                    }
                    else if (sceneAnimalPair.second.WhatKindOfAnimalIsThis() == "Horse")
                    {
                        healthMultiplier = 1.5; // faster health decay
                    }
                        // Seal and Owl remain at normal decay (1.0)
                    
                    // Keep sanity static across non-active scenes.
                    // sceneAnimalPair.second.current_sanity -= sceneAnimalPair.second.sanityDecayRate * static_cast<double>(dt);

                    

                    if (sceneAnimalPair.second.current_hunger <= 0 || sceneAnimalPair.second.current_health <= 0 || sceneAnimalPair.second.current_sanity <= 0) {
                        sceneAnimalPair.second.is_alive = false;
                        
                        deadAnimals++;
                    }
                
                    LogLowHealth(sceneAnimalPair);
                }
            }

            if (deadAnimals>=deathLimit)
            {
                LoseGame();
            }
            gameTimeElapsed += dt;
            if (gameTimeElapsed>=winRequirement)
            {
                WinGame();
            }
        }
    }

        
    

    AnimalAttributes GameManager::LoadPersistentValues(GameScene gs) {
        targetVisibility = false;
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
    void GameManager::ResetSystem(bool editorMode) {
		UNREFERENCED_PARAMETER(editorMode);

        AnimalAttributes defaultAttributes{};
        defaultAttributes.SetDefault();
        for (std::pair<const GameScene, AnimalAttributes>& sceneAnimalPair : sceneAnimalMap)
        {
            sceneAnimalPair.second = defaultAttributes;
            if (sceneAnimalPair.first == GameScene::OWL) {
                sceneAnimalPair.second.hungerDecayRate = 1.5f;//owl special hunger
            }
        }
        gameTimeElapsed = 0.f;
        diegeticLogEntries.clear();
        deadAnimals = 0;
        currState = GameManager::GameState::ENDING;
        targetVisibility = false;
    }

    void GameManager::SaveAnimalAttributesToCurrentScene(AnimalAttributes aA) {
        if (currState == GameManager::GameState::ENDING)return;
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

    void GameManager::LoseGame() {
        ResetSystem(false);
        p_Scene->setSceneToLoad("Assets/Scene/Game_Over.json");

    }
    void GameManager::WinGame() {
        ResetSystem();
        p_Scene->setSceneToLoad("Assets/Scene/Menu.json");

    }

    bool GameManager::HasLogBeenUpdated() {

        if (!diegeticLogUpdated) {
            diegeticLogUpdated = true;
            return false;

        }
        else
        {
            return true;
        }
    }
    
    void GameManager::AddLogEntry(std::string message) {
        float timeInGameWorld{ gameTimeElapsed * GetTimeScale()};
        int hours{ static_cast<int>(timeInGameWorld / 3600.f) },
            mins{ static_cast<int>((timeInGameWorld - (static_cast<float>(hours) * 3600.f)) / 60.f) },
            secs{ static_cast<int>(timeInGameWorld - (static_cast<float>(hours) * 3600.f) - (static_cast<float>(mins) * 60.f)) };

        char ingameTime[16];
        sprintf(ingameTime, "[%02d:%02d:%02d]", hours, mins, secs);
        
        std::string fullMessage{ ingameTime };
        fullMessage += " " + message;
        //std::cout << fullMessage << " logged\n";
        
        if (diegeticLogEntries.size()>=maxDiegeticLogEntries)
        {
            diegeticLogEntries.pop_front();

        }
        diegeticLogEntries.push_back(fullMessage);
        
        diegeticLogUpdated = false;
    }

    void GameManager::LogLowHealth(std::pair<const GameScene, AnimalAttributes>& sceneAnimalPair) {
        std::string animalName{};

        switch (sceneAnimalPair.first)
        {
        case GameScene::OWL: animalName = "Owl";
            break;
        case GameScene::FLAMINGO:animalName = "Flamingo";
            break;
        case GameScene::HORSE:animalName = "Horse";
            break;
        case GameScene::SEAL:animalName = "Seal";
            break;
        }


        if (sceneAnimalPair.second.current_health <= sceneAnimalPair.second.maxHealth / 2.0 && !sceneAnimalPair.second.healthWarn)
        {
            AddLogEntry("WARNING! " +animalName + "'s Health is below 50%!");
            sceneAnimalPair.second.healthWarn = true;
        }
        else if (sceneAnimalPair.second.current_health > sceneAnimalPair.second.maxHealth / 2.0 && sceneAnimalPair.second.healthWarn) {
            sceneAnimalPair.second.healthWarn = false;
        }
        if (sceneAnimalPair.second.current_hunger <= sceneAnimalPair.second.maxHunger / 2.0 && !sceneAnimalPair.second.hungerWarn)
        {
            AddLogEntry("WARNING! " + animalName + "'s Hunger is below 50%!");
            sceneAnimalPair.second.hungerWarn = true;
        }
        else if (sceneAnimalPair.second.current_hunger > sceneAnimalPair.second.maxHunger / 2.0 && sceneAnimalPair.second.hungerWarn) {
            sceneAnimalPair.second.hungerWarn = false;
        }
        if (sceneAnimalPair.second.current_sanity <= sceneAnimalPair.second.maxSanity / 2.0 && !sceneAnimalPair.second.sanityWarn)
        {
            AddLogEntry("WARNING! " + animalName + "'s Sanity is below 50%!");
            sceneAnimalPair.second.sanityWarn = true;
        }
        else if (sceneAnimalPair.second.current_sanity > sceneAnimalPair.second.maxSanity / 2.0 && sceneAnimalPair.second.sanityWarn) {
            sceneAnimalPair.second.sanityWarn = false;
        }

        if (!sceneAnimalPair.second.is_alive)
        {
            AddLogEntry(animalName + " is dead.");
        }
    }

    void GameManager::CombinedSceneUpdate(float dt) {

    }
}
