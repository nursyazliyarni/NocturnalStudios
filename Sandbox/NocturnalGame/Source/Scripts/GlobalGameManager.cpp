/*!------------------------------------------------------------------------------------------------
    File: GlobalGameManager.cpp
    Project: NocturnalStudio

    Brief:
        This file tracks the logic of animals across scenes and also runs the game loop. 

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the GameManager class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/GlobalGameManager.h"
#include "ECS/ECS.h"
#include "GUI/GUI.h"
#include "ECS/SceneManager.h"
#include "core/Log.h"

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
    GameManager::GameManager() :gameTimeElapsed{ 0.f }, currScene{ "" }, sceneAnimalMap{}
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

        sceneAnimalMap.emplace("Flamingo", defaultAttributes);
        sceneAnimalMap["Flamingo"].animalType = AnimalAttributes::AnimalType::FLAMINGO;
        sceneAnimalMap.emplace("Horse", defaultAttributes);
        sceneAnimalMap["Horse"].animalType = AnimalAttributes::AnimalType::HOESE;
        sceneAnimalMap.emplace("Owl", defaultAttributes);
        sceneAnimalMap["Owl"].animalType = AnimalAttributes::AnimalType::OWL;
        sceneAnimalMap["Owl"].hungerDecayRate = 1.5; //owl special hunger
        sceneAnimalMap.emplace("Seal", defaultAttributes);
        sceneAnimalMap["Seal"].animalType = AnimalAttributes::AnimalType::SEAL;
        currState=  GameManager::GameState::PLAYING;
        deadAnimals = 0;
    }

    void GameManager::CombinedSceneStart() {
        
        animalE = GetEntityByName("Animal");
        cageBGE = GetEntityByName("BGObject");
        containmentE[0] = GetEntityByName("AnimalName");
        containmentE[1] = GetEntityByName("EnclosureTitle");
        containmentE[2] = GetEntityByName("Intro");

        p_ecs.GetComponent<Audio>(cageBGE).GetArr().clear();

        const std::string animalNames[]{ "Flamingo","Horse","Owl","Seal" };
        int audI{ 0 };
        for (std::string name:animalNames)
        {
            Entity animal{ GetEntityByName(name) }, animalBG{ GetEntityByName(name + "BG") };
            if (animal==0||animalBG==0)
            {
                NOCTURNAL_GAME_WARN("Animal or AnimalBG missing for: " + name);
                break;
            }

            if (!p_ecs.HaveComponent<Sprite>(animal)|| 
                !p_ecs.HaveComponent<Transform>(animal)|| 
                !p_ecs.HaveComponent<Collider>(animal)|| 
                !p_ecs.HaveComponent<Text>(animal)||
                !p_ecs.HaveComponent<AnimalAttributes>(animal))
            {
                NOCTURNAL_GAME_WARN("Animal " + name+" entity is missing one or more of the required components!");
                break;
            }

            if (!p_ecs.HaveComponent<Sprite>(animalBG) ||
                !p_ecs.HaveComponent<Audio>(animalBG) )
            {
                NOCTURNAL_GAME_WARN("Animal " + name + "BG entity is missing one or more of the required components!");
                break;
            }

            AnimalSceneData asd;
            
            asd.aTransform = p_ecs.GetComponent<Transform>(animal);
            asd.aCollider = p_ecs.GetComponent<Collider>(animal);
            asd.aniSprite = p_ecs.GetComponent<Sprite>(animal);
            asd.bgSprite = p_ecs.GetComponent<Sprite>(animalBG);

            p_ecs.GetComponent<Audio>(cageBGE).GetArr().push_back (p_ecs.GetComponent<Audio>(animalBG).GetArr()[0]);
            asd.bgAudioInd = audI;
            audI++;
            
            std::string toSplit{ p_ecs.GetComponent<Text>(animal).GetContent()};

            for (std::string& str:asd.containmentEntries)
            {
                if (toSplit.empty())
                {
                    str.clear();
                    continue;
                }
                size_t pos = toSplit.find_first_of(':');

                if (pos == std::string::npos)   // last token
                {
                    str = toSplit;
                    toSplit.clear();
                }
                else
                {
                    str = toSplit.substr(0, pos);
                    toSplit.erase(0, pos + 1);
                }
            }

            sceneAnimalMap[name] = p_ecs.GetComponent<AnimalAttributes>(animal);

            p_ecs.DestroyEntity(animal);
            p_ecs.DestroyEntity(animalBG);
            
            combinedSceneMap.emplace(name, asd);
        }



        currScene = "Flamingo";

        combinedSceneSetup = true;
    }
    /*!*************************************************************************
    This function runs the logic of the system. It checks the list of entities with
    a collision component in pairs to see which of them are colliding. If two objects
    are colliding, collision response is dictated by the type of collider the object
    has
    ****************************************************************************/
    void GameManager::Update(float dt) {
       
        

        if (p_Scene->isGameplay)
        {
            
            if (IsNewSceneManagement() && !combinedSceneSetup) CombinedSceneStart();
            if (IsNewSceneManagement())CombinedSceneUpdate(dt);
            for (std::pair<const std::string,AnimalAttributes>& sceneAnimalPair:sceneAnimalMap)
            {
                //this processes the other animals in the 
                // background to give the illusion of persistent data
                //For the animal currently in scene, refer to Scripts/AnimalLogic.cpp
                // The code should be the exact same
                if (sceneAnimalPair.first!=currScene&& !sceneAnimalPair.second.isEscaped &&sceneAnimalPair.second.is_alive)
                {
                    sceneAnimalPair.second.current_hunger -= sceneAnimalPair.second.hungerDecayRate * static_cast<double>( dt) * 2.0;
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
                    sceneAnimalPair.second.current_health -= sceneAnimalPair.second.healthDecayRate * static_cast<double>(dt) * healthMultiplier;
                    sceneAnimalPair.second.current_sanity -= sceneAnimalPair.second.sanityDecayRate * static_cast<double>(dt);

                    
                    if (sceneAnimalPair.second.current_sanity <= 0)
                    {
                        sceneAnimalPair.second.isEscaped = true;
                        escapedAnimals++;
                    }
                    if (sceneAnimalPair.second.current_hunger <= 0 || sceneAnimalPair.second.current_health <= 0) {
                        sceneAnimalPair.second.is_alive = false;
                        
                        deadAnimals++;
                    }
                
                    LogLowHealth(sceneAnimalPair);
                }
            }

            if (deadAnimals+escapedAnimals>=deathLimit)
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

        
    

    AnimalAttributes GameManager::LoadPersistentValues(std::string gs) {
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
    void GameManager::ResetSystem(bool resetStats) {


        AnimalAttributes defaultAttributes{};
        defaultAttributes.SetDefault();
        for (std::pair<const std::string, AnimalAttributes>& sceneAnimalPair : sceneAnimalMap)
        {
            sceneAnimalPair.second = defaultAttributes;
            if (sceneAnimalPair.first == "Owl") {
                sceneAnimalPair.second.hungerDecayRate = 1.5f;//owl special hunger
            }
        }
        gameTimeElapsed = 0.f;
        diegeticLogEntries.clear();
        if (resetStats)
        {
            deadAnimals = 0;
            escapedAnimals = 0;
            under20 = 0;
        }
        
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
        p_Scene->setSceneToLoad("Assets/Scene/LoseScene.json");

    }
    void GameManager::WinGame() {
        ResetSystem(false);
        p_Scene->setSceneToLoad("Assets/Scene/WinScene.json");

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

    void GameManager::LogLowHealth(std::pair<const std::string, AnimalAttributes>& sceneAnimalPair) {
        


        if (sceneAnimalPair.second.current_health <= sceneAnimalPair.second.maxHealth / 2.0 && sceneAnimalPair.second.healthWarnLevel < 1)
        {
            AddLogEntry("WARNING! " + sceneAnimalPair.first + "'s Health is below 50%!");
            sceneAnimalPair.second.healthWarnLevel++;
        }
        else if (sceneAnimalPair.second.current_health <= sceneAnimalPair.second.maxHealth / 5.0 && sceneAnimalPair.second.healthWarnLevel < 2)
        {
            AddLogEntry("WARNING! " + sceneAnimalPair.first + "'s Health is below 20%!");
            sceneAnimalPair.second.healthWarnLevel++;
            AddCrit();
        }
        else if (sceneAnimalPair.second.current_health > sceneAnimalPair.second.maxHealth / 2.0 && sceneAnimalPair.second.healthWarnLevel != 0) {
            sceneAnimalPair.second.healthWarnLevel = 0;
        }
        if (sceneAnimalPair.second.current_hunger <= sceneAnimalPair.second.maxHunger / 2.0 && sceneAnimalPair.second.hungerWarnLevel < 1)
        {
            AddLogEntry("WARNING! " + sceneAnimalPair.first + "'s Hunger is below 50%!");
            sceneAnimalPair.second.hungerWarnLevel++;
        }
        else if (sceneAnimalPair.second.current_hunger <= sceneAnimalPair.second.maxHunger / 5.0 && sceneAnimalPair.second.hungerWarnLevel < 2)
        {
            AddLogEntry("WARNING! " + sceneAnimalPair.first + "'s Hunger is below 20%!");
            sceneAnimalPair.second.hungerWarnLevel++;
            AddCrit();
        }
        else if (sceneAnimalPair.second.current_hunger > sceneAnimalPair.second.maxHunger / 2.0 && sceneAnimalPair.second.hungerWarnLevel != 0) {
            sceneAnimalPair.second.hungerWarnLevel = 0;
        }
        if (sceneAnimalPair.second.current_sanity <= sceneAnimalPair.second.maxSanity / 2.0 && sceneAnimalPair.second.sanityWarnLevel < 1)
        {
            AddLogEntry("WARNING! " + sceneAnimalPair.first + "'s Sanity is below 50%!");
            sceneAnimalPair.second.sanityWarnLevel++;
        }
        else if (sceneAnimalPair.second.current_sanity <= sceneAnimalPair.second.maxSanity / 5.0 && sceneAnimalPair.second.sanityWarnLevel < 2)
        {
            AddLogEntry("WARNING! " + sceneAnimalPair.first + "'s Sanity is below 20%!");
            sceneAnimalPair.second.sanityWarnLevel++;
            AddCrit();
        }
        else if (sceneAnimalPair.second.current_sanity > sceneAnimalPair.second.maxSanity / 2.0 && sceneAnimalPair.second.sanityWarnLevel != 0) {
            sceneAnimalPair.second.sanityWarnLevel = 0;
        }

        if (!sceneAnimalPair.second.is_alive)
        {
            AddLogEntry(sceneAnimalPair.first + " is dead.");
        }
        if (sceneAnimalPair.second.isEscaped)
        {
            AddLogEntry(sceneAnimalPair.first + " has breached containment.");
        }
    }

    void GameManager::CombinedSceneUpdate(float dt) {
        UNREFERENCED_PARAMETER(dt);
        if (innerState==2) //ending, save animaldata to the corresponding array
        {
            sceneAnimalMap[currScene] = p_ecs.GetComponent<AnimalAttributes>(animalE);
            p_ecs.GetComponent<Audio>(cageBGE).GetArr()[combinedSceneMap[currScene].bgAudioInd].is_Playing = false;
            currScene = nextScene;
            innerState = 0;
        }
        if (innerState == 0) 
        {
            auto it = combinedSceneMap.find(currScene);
            if (it!=combinedSceneMap.end())
            {
                p_ecs.GetComponent<Transform>(animalE) = it->second.aTransform;
                p_ecs.GetComponent<Sprite>(animalE) = it->second.aniSprite;
                p_ecs.GetComponent<Collider>(animalE) = it->second.aCollider;
                p_ecs.GetComponent<AnimalAttributes>(animalE) = sceneAnimalMap[currScene];

                p_ecs.GetComponent<Sprite>(cageBGE) = it->second.bgSprite;
                
                p_ecs.GetComponent<Audio>(cageBGE).GetArr()[it->second.bgAudioInd].is_Playing=true;

                for (int i = 0; i < 3; i++)
                {
                    p_ecs.GetComponent<Text>(containmentE[i]).SetContent(it->second.containmentEntries[i]);
                }
            }
            innerState = 1;
        }
        if (innerState==1)
        {
            
        }

    }

    Entity GameManager::GetEntityByName(const std::string name) {
        std::set<Entity> eSet{ p_ecs.GetAliveEntitiesSet() };

        for (Entity e:eSet)
        {
            if (p_ecs.HaveComponent<NameTag>(e) && p_ecs.GetComponent<NameTag>(e).GetNameTag() == name) return e;
        }
        return 0;
    }

    void GameManager::SetSceneToSwitch( std::string seekScene ){

        //std::cout << "switch requested to " << seekScene;
        if (seekScene!=currScene)
        {
            nextScene = seekScene;
            innerState = 2;
        }
    }
}
