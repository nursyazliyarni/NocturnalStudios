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
#include "Math/Random.h"
#include "Scripts/AnimalLogic.h"

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
        sceneAnimalMap["Flamingo"].aA.SetAnimalType(AnimalAttributes::AnimalType::FLAMINGO);
        sceneAnimalMap.emplace("Horse", defaultAttributes);
        sceneAnimalMap["Horse"].aA.SetAnimalType(AnimalAttributes::AnimalType::HOESE);
        sceneAnimalMap.emplace("Owl", defaultAttributes);
        sceneAnimalMap["Owl"].aA.SetAnimalType(AnimalAttributes::AnimalType::OWL);
        sceneAnimalMap["Owl"].aA.hungerDecayRate = 1.5; //owl special hunger
        sceneAnimalMap.emplace("Seal", defaultAttributes);
        sceneAnimalMap["Seal"].aA.SetAnimalType(AnimalAttributes::AnimalType::SEAL);
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

            sceneAnimalMap[name] = { p_ecs.GetComponent<AnimalAttributes>(animal) };

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
            for (std::pair<const std::string,AnimalData>& sceneAnimalPair:sceneAnimalMap)
            {
                //this processes the other animals in the 
                // background to give the illusion of persistent data
                //For the animal currently in scene, refer to Scripts/AnimalLogic.cpp
                // The code should be the exact same
                if (sceneAnimalPair.first!=currScene&& !sceneAnimalPair.second.aA.isEscaped &&sceneAnimalPair.second.aA.is_alive)
                {
                    
                        //hunger>hp>sanity
                        CheckAndDecreaseStats(sceneAnimalPair.second.hungerT, sceneAnimalPair.second.hungerD, sceneAnimalPair.second.aA.hungerDecayRate, sceneAnimalPair.second.aA.current_hunger, dt);

                        double hpDecay = sceneAnimalPair.second.aA.healthDecayRate, decreaseScale = 1.0;
                        if (sceneAnimalPair.second.aA.WhatKindOfAnimalIsThis() == "Flamingo")
                        {
                            if (sceneAnimalPair.second.aA.current_hunger > sceneAnimalPair.second.aA.maxHunger * 0.7) { hpDecay *= 0.5; decreaseScale = 0.7; }
                            else if (sceneAnimalPair.second.aA.current_hunger <= sceneAnimalPair.second.aA.maxHunger * 0.3) { hpDecay *= 2.0; decreaseScale = 2.0; }
                        }
                        if (sceneAnimalPair.second.aA.WhatKindOfAnimalIsThis() == "Horse")
                        {
                            if (sceneAnimalPair.second.aA.current_hunger <= sceneAnimalPair.second.aA.maxHunger * 0.8) { hpDecay *= 1.5; decreaseScale = 1.5; }
                        }

                        CheckAndDecreaseStats(sceneAnimalPair.second.hpT, sceneAnimalPair.second.hpD, hpDecay, sceneAnimalPair.second.aA.current_health, dt);

                        sceneAnimalPair.second.aA.current_sanity -= sceneAnimalPair.second.aA.sanityDecayRate * static_cast<double>(dt);

                        if (sceneAnimalPair.second.aA.current_sanity <= 0)
                        {
                            sceneAnimalPair.second.aA.isEscaped = true;

                            p_GameManager.AddEscape();
                        }
                        if (sceneAnimalPair.second.aA.current_hunger <= 0 || sceneAnimalPair.second.aA.current_health <= 0) {
                            sceneAnimalPair.second.aA.is_alive = false;
                            p_GameManager.AddDeath();
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

        
    

    GameManager::AnimalData GameManager::LoadPersistentValues(std::string gs) {
        targetVisibility = false;
        auto it = sceneAnimalMap.find(gs);
        if (it != sceneAnimalMap.end()) {
            currScene = gs;
            return it->second;
        }
        else {
            AnimalAttributes defaultAttributes{};
            defaultAttributes.SetDefault();
            AnimalData da{ defaultAttributes };
            return da;
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
        for (std::pair<const std::string, AnimalData>& sceneAnimalPair : sceneAnimalMap)
        {
            sceneAnimalPair.second.aA = defaultAttributes;
            if (sceneAnimalPair.first == "Owl") {
                sceneAnimalPair.second.aA.hungerDecayRate = 1.5f;//owl special hunger
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

    void GameManager::SaveAnimalAttributesToCurrentScene(AnimalData aA) {
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

    void GameManager::LogLowHealth(std::pair<const std::string, AnimalData>& sceneAnimalPair) {
        


        if (sceneAnimalPair.second.aA.current_health <= sceneAnimalPair.second.aA.maxHealth / 2.0 && sceneAnimalPair.second.aA.healthWarnLevel < 1)
        {
            //AddLogEntry("WARNING! " + sceneAnimalPair.first + "'s Health is below 50%!");
            sceneAnimalPair.second.aA.healthWarnLevel++;
        }
        else if (sceneAnimalPair.second.aA.current_health <= sceneAnimalPair.second.aA.maxHealth / 5.0 && sceneAnimalPair.second.aA.healthWarnLevel < 2)
        {
            p_GameManager.AddLogEntry("Body Temperature of " + sceneAnimalPair.first + " is critical");
            //AddLogEntry("WARNING! " + sceneAnimalPair.first + "'s Health is below 20%!");
            sceneAnimalPair.second.aA.healthWarnLevel++;
            AddCrit();
        }
        else if (sceneAnimalPair.second.aA.current_health > sceneAnimalPair.second.aA.maxHealth / 2.0 && sceneAnimalPair.second.aA.healthWarnLevel != 0) {
            sceneAnimalPair.second.aA.healthWarnLevel = 0;
        }
        if (sceneAnimalPair.second.aA.current_hunger <= sceneAnimalPair.second.aA.maxHunger / 2.0 && sceneAnimalPair.second.aA.hungerWarnLevel < 1)
        {
            //AddLogEntry("WARNING! " + sceneAnimalPair.first + "'s Hunger is below 50%!");
            sceneAnimalPair.second.aA.hungerWarnLevel++;
        }
        else if (sceneAnimalPair.second.aA.current_hunger <= sceneAnimalPair.second.aA.maxHunger / 5.0 && sceneAnimalPair.second.aA.hungerWarnLevel < 2)
        {
            p_GameManager.AddLogEntry(sceneAnimalPair.first + " has not been fed for " + std::to_string(Random::Range(60, 180)) + " minutes");
            //AddLogEntry("WARNING! " + sceneAnimalPair.first + "'s Hunger is below 20%!");
            sceneAnimalPair.second.aA.hungerWarnLevel++;
            AddCrit();
        }
        else if (sceneAnimalPair.second.aA.current_hunger > sceneAnimalPair.second.aA.maxHunger / 2.0 && sceneAnimalPair.second.aA.hungerWarnLevel != 0) {
            sceneAnimalPair.second.aA.hungerWarnLevel = 0;
        }
        if (sceneAnimalPair.second.aA.current_sanity <= sceneAnimalPair.second.aA.maxSanity / 2.0 && sceneAnimalPair.second.aA.sanityWarnLevel < 1)
        {
            //AddLogEntry("WARNING! " + sceneAnimalPair.first + "'s Sanity is below 50%!");
            sceneAnimalPair.second.aA.sanityWarnLevel++;
        }
        else if (sceneAnimalPair.second.aA.current_sanity <= sceneAnimalPair.second.aA.maxSanity / 5.0 && sceneAnimalPair.second.aA.sanityWarnLevel < 2)
        {
            p_GameManager.AddLogEntry("Banging noises detected from " + sceneAnimalPair.first + "'s enclosure");
            //AddLogEntry("WARNING! " + sceneAnimalPair.first + "'s Sanity is below 20%!");
            sceneAnimalPair.second.aA.sanityWarnLevel++;
            AddCrit();
        }
        else if (sceneAnimalPair.second.aA.current_sanity > sceneAnimalPair.second.aA.maxSanity / 2.0 && sceneAnimalPair.second.aA.sanityWarnLevel != 0) {
            sceneAnimalPair.second.aA.sanityWarnLevel = 0;
        }

        if (!sceneAnimalPair.second.aA.is_alive)
        {
            AddLogEntry(sceneAnimalPair.first + " is not moving");
        }
        if (sceneAnimalPair.second.aA.isEscaped)
        {
           AddLogEntry(sceneAnimalPair.first + " has breached containment.");
        }
    }

    void GameManager::CombinedSceneUpdate(float dt) {
        UNREFERENCED_PARAMETER(dt);
        if (innerState==2) //ending, save animaldata to the corresponding array
        {
            sceneAnimalMap[currScene].aA = p_ecs.GetComponent<AnimalAttributes>(animalE);
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
                p_ecs.GetComponent<AnimalAttributes>(animalE) = sceneAnimalMap[currScene].aA;

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

    void GameManager::CheckAndDecreaseStats(float& time, float& chance, double decayRate, double& attributeValue, float frametime, double decreaseScale) {
        if (decayRate <= 0.0)decayRate = 1.0;
        if (time >= (winRequirement / 72.f) / decayRate)
        {
            //std::cout << "Checking\n";
            float bruh{ Random::Range(0.f, 100.f) };
            if (bruh <= chance)
            {
                //std::cout << bruh << "\n";
                attributeValue -= Random::Range(5.f, 15.f) * decreaseScale;
                chance = 10.f;
            }
            else {
                chance += 10.f;
            }
            time = 0.f;
        }
        else
        {
            time += frametime;
        }
    }
}
