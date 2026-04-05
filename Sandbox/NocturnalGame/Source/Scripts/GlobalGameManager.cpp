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
#include "Scripts/EncryptionKey.h"
#include "ECS/Components/ButtonComponent.h"
#include "Audio/AudioEngine.h"
#include "graphics/Graphics.h"

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
        Graphic::layerEnabled[9] = false; //hide pause layer on start
        p_GUI->set_pause(false);
        if (hasInitialized) return; //already set up, don't reinitialize
        hasInitialized = true;

        if (!levelConfig.SetupLevelData("Assets/LevelPreset")||levelConfig.GetLevels().find(levelNum)==levelConfig.GetLevels().end()) {
            AnimalAttributes defaultAttributes{};
            defaultAttributes.SetDefault();

            sceneAnimalMap.emplace("Flamingo", defaultAttributes);
            sceneAnimalMap["Flamingo"].aA.SetAnimalType(AnimalAttributes::AnimalType::FLAMINGO);
            sceneAnimalMap.emplace("Seal", defaultAttributes);
            sceneAnimalMap["Seal"].aA.SetAnimalType(AnimalAttributes::AnimalType::SEAL);
            sceneAnimalMap.emplace("Horse", defaultAttributes);
            sceneAnimalMap["Horse"].aA.SetAnimalType(AnimalAttributes::AnimalType::HOESE);
            sceneAnimalMap.emplace("Owl", defaultAttributes);
            sceneAnimalMap["Owl"].aA.SetAnimalType(AnimalAttributes::AnimalType::OWL);
            sceneAnimalMap["Owl"].aA.hungerDecayRate = 1.5; //owl special hunger
            levelDataLoaded = true;
        }
        else {
            LevelDataToMap(levelNum);
        }

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
       
        if (!IsPaused())
        {
            Graphic::layerEnabled[9] = false;
        }

        if (p_Scene->isGameplay&& currState !=GameState::PAUSED)
        {

            
            if (!levelDataLoaded && !levelConfig.GetLevels().empty())LevelDataToMap(levelNum);


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
                    if (levelNum <= 1 && sceneAnimalPair.first == "Owl")continue;
                    sceneAnimalPair.second.aA.timeSinceLastFeed += dt;
                        sceneAnimalPair.second.aA.timeSinceSanity += dt;

                        if (!sceneAnimalPair.second.toy.isHiding) sceneAnimalPair.second.toy.isHiding = CheckHide(sceneAnimalPair.second.aA);
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

                        CheckAndDecreaseStats(sceneAnimalPair.second.hpT, sceneAnimalPair.second.hpD, hpDecay, sceneAnimalPair.second.aA.current_health, dt,decreaseScale);

                        if (sceneAnimalPair.second.toy.isHiding)
                        {
                            sceneAnimalPair.second.aA.current_sanity -= sceneAnimalPair.second.aA.sanityDecayRate * static_cast<double>(dt);
                        }

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

        
    bool GameManager::LevelDataToMap(int levelN) {
        const std::map<int, LevelConfig::LevelData>& lvlDataMap = levelConfig.GetLevels();
        if (lvlDataMap.empty() || lvlDataMap.find(levelN) == lvlDataMap.end())return false;

        const LevelConfig::LevelData& leveldat = lvlDataMap.at(levelN);
        sceneAnimalMap.clear();
        for (const std::pair<const LevelConfig::AnimalRef, AnimalAttributes>& animal : leveldat)
        {
            AnimalData aD{};
            aD.aA = animal.second;
            switch (animal.first)
            {
            case LevelConfig::AnimalRef::FLAMINGO:
                sceneAnimalMap.emplace("Flamingo", aD);
                break;
            case LevelConfig::AnimalRef::SEAL:
                sceneAnimalMap.emplace("Seal", aD);
                break;
            case LevelConfig::AnimalRef::HORSE:
                sceneAnimalMap.emplace("Horse", aD);
                break;
            case LevelConfig::AnimalRef::OWL:
                sceneAnimalMap.emplace("Owl", aD);
                break;
            }
        }
        levelDataLoaded = true;
        return true;
    }

    GameManager::AnimalData GameManager::LoadPersistentValues(const std::string& gs) {
        if (levelNum<=1)
        {
            Entity owlBtn{ GetEntityByName("OwlBtn") };
            if (owlBtn != MAX_ENTITIES && p_ecs.HaveComponent<ButtonComponent>(owlBtn))
            {
                p_ecs.GetComponent<ButtonComponent>(owlBtn).ClearFlag(); //Deactivates the button
                if (p_ecs.HaveComponent<Text>(owlBtn))p_ecs.GetComponent<Text>(owlBtn).SetContent("");
            }
        }

        Entity inventoryBtn{ GetEntityByName("InventoryBtn") };
        if (inventoryBtn != MAX_ENTITIES && p_ecs.HaveComponent<ButtonComponent>(inventoryBtn))
        {
            p_ecs.GetComponent<ButtonComponent>(inventoryBtn).SET_button_logic_ON(
                levelConfig.HasExtraScene(static_cast<int>(levelNum), "hiddenending"));
        }

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

    std::string GameManager::CanMoveScene(const std::string& sceneName, bool dir) {
        auto idk = sceneAnimalMap.find(sceneName);
        if (idk == sceneAnimalMap.end())return "";
        //true left false right
        if (dir) {
            if(sceneName == "Flamingo")return "";
            else {
                idk--;
                return idk->first;
            }

        }
        if (!dir) {
            if(sceneName == "Owl" || levelNum == 1 && sceneName == "Horse")return "";
            else {
                idk++;
                return idk->first;
            }
        }

        
    }

    GameManager::ToyState GameManager::LoadToyState(const std::string& gs) {
        auto it = sceneAnimalMap.find(gs);
        if (it != sceneAnimalMap.end()) {
            //currScene = gs;
            return it->second.toy;
        }
        else {

            return {};
        }
    }
    /*!*************************************************************************
    This function ends the system. As there are no data members that require
    initialization, there are no data members that need to be un-initialised.
    Therefore this function is empty
    ****************************************************************************/
    void GameManager::ResetSystem(bool resetStats) {
        Graphic::layerEnabled[9] = false;
        p_GUI->set_pause(false);
        EncryptionKey::ResetRunState();

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
        
        currState = GameManager::GameState::PLAYING;
        targetVisibility = false;
        if (!hasInitialized)
        {
            hasInitialized = true;
        }
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
    void GameManager::SaveToyState(bool hiding, int hideIndex) {
        if (currState == GameManager::GameState::ENDING)return;
        auto it = sceneAnimalMap.find(currScene);

        //there is an older version of the aa in the thingy
        //oh i have to rekmember to iterate this with one by default ops
        if (it != sceneAnimalMap.end()) {
            it->second.toy = { hiding,hideIndex };
        }
    }
    void GameManager::NextLevel() {
        unsigned int maxlvls = std::min(levelCount, static_cast<unsigned int>(levelConfig.GetLevels().size()));
        p_GameManager.ResetStats();
        levelDataLoaded = false;
        if (levelNum<maxlvls)
        {
            levelNum++;
            LevelDataToMap(levelNum);
            p_Scene->setSceneToLoad("Assets/Scene/FlamingoScene.json");// restart the game in pause
            SetState(GameManager::GameState::PLAYING);
        }
        else {
            levelNum = 1;
            p_Scene->setSceneToLoad("Assets/Scene/Menu.json");//reset the game level in game over
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
		p_Audio->PlaySound("Assets/Audio/Alert_v2.wav", Audio::AudioType::SFX);
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
            sceneAnimalPair.second.aA.healthWarnLevel++;
        }
        else if (sceneAnimalPair.second.aA.current_health <= sceneAnimalPair.second.aA.maxHealth / 5.0 && sceneAnimalPair.second.aA.healthWarnLevel < 2)
        {
            AddLogEntry(sceneAnimalPair.first + "'s body temperature is alarmingly high.");
            sceneAnimalPair.second.aA.healthWarnLevel++;
            AddCrit();
        }
        else if (sceneAnimalPair.second.aA.current_health > sceneAnimalPair.second.aA.maxHealth / 2.0 && sceneAnimalPair.second.aA.healthWarnLevel != 0) {
            sceneAnimalPair.second.aA.healthWarnLevel = 0;
        }
        if (sceneAnimalPair.second.aA.current_hunger <= sceneAnimalPair.second.aA.maxHunger / 2.0 && sceneAnimalPair.second.aA.hungerWarnLevel < 1)
        {
            int minutesPassed = static_cast<int>((GetTimeScale() * sceneAnimalPair.second.aA.timeSinceLastFeed) / 60.f);
            if (minutesPassed < 60) p_GameManager.AddLogEntry(sceneAnimalPair.first + " seems hungry...");
            else AddLogEntry(sceneAnimalPair.first + " has not been fed for " + std::to_string(minutesPassed) + " minutes.");
            sceneAnimalPair.second.aA.hungerWarnLevel++;
        }
        else if (sceneAnimalPair.second.aA.current_hunger <= sceneAnimalPair.second.aA.maxHunger / 5.0 && sceneAnimalPair.second.aA.hungerWarnLevel < 2)
        {
            AddLogEntry(sceneAnimalPair.first + " is starving!");
            sceneAnimalPair.second.aA.hungerWarnLevel++;
            AddCrit();
        }
        else if (sceneAnimalPair.second.aA.current_hunger > sceneAnimalPair.second.aA.maxHunger / 2.0 && sceneAnimalPair.second.aA.hungerWarnLevel != 0) {
            sceneAnimalPair.second.aA.hungerWarnLevel = 0;
        }
        if (sceneAnimalPair.second.aA.current_sanity <= sceneAnimalPair.second.aA.maxSanity / 2.0 && sceneAnimalPair.second.aA.sanityWarnLevel < 1)
        {
            AddLogEntry(sceneAnimalPair.first + " is searching for something...");
            sceneAnimalPair.second.aA.sanityWarnLevel++;
        }
        else if (sceneAnimalPair.second.aA.current_sanity <= sceneAnimalPair.second.aA.maxSanity / 5.0 && sceneAnimalPair.second.aA.sanityWarnLevel < 2)
        {
            AddLogEntry(sceneAnimalPair.first + " is exhibiting increased restlessness.");
            sceneAnimalPair.second.aA.sanityWarnLevel++;
            AddCrit();
        }
        else if (sceneAnimalPair.second.aA.current_sanity > sceneAnimalPair.second.aA.maxSanity / 2.0 && sceneAnimalPair.second.aA.sanityWarnLevel != 0) {
            sceneAnimalPair.second.aA.sanityWarnLevel = 0;
        }

        if (!sceneAnimalPair.second.aA.is_alive)
        {
            AddLogEntry("Please check "+sceneAnimalPair.first+"'s enclosure. ");
            AddLogEntry(sceneAnimalPair.first + " is not moving.");
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
    bool GameManager::CheckHide(AnimalAttributes& aa)
    {
        if (aa.sanityDecayRate <= 0.0)aa.sanityDecayRate = 0.00000001;

        if (aa.timeSinceSanity >= GetCheckPeriod() / aa.sanityDecayRate)
        {
            aa.timeSinceSanity = 0.f;

            if (Random::Range(0, 1))
            {
                return true;

            }
        }
        return false;
    }
    Entity GameManager::GetEntityByName(const std::string name) {
        std::set<Entity> eSet{ p_ecs.GetAliveEntitiesSet() };

        for (Entity e:eSet)
        {
            if (p_ecs.HaveComponent<NameTag>(e) && p_ecs.GetComponent<NameTag>(e).GetNameTag() == name) return e;
        }
        return MAX_ENTITIES;
    }

    void GameManager::SetSceneToSwitch( std::string seekScene ){

        //std::cout << "switch requested to " << seekScene;
        if (seekScene!=currScene)
        {
            nextScene = seekScene;
            innerState = 2;
        }
    }
    void GameManager::AllFullStats() {
        if (!LevelDataToMap(levelNum)) {
            AnimalAttributes defaultAttributes{};
            defaultAttributes.SetDefault();

            sceneAnimalMap.emplace("Flamingo", defaultAttributes);
            sceneAnimalMap["Flamingo"].aA.SetAnimalType(AnimalAttributes::AnimalType::FLAMINGO);
            sceneAnimalMap.emplace("Seal", defaultAttributes);
            sceneAnimalMap["Seal"].aA.SetAnimalType(AnimalAttributes::AnimalType::SEAL);
            sceneAnimalMap.emplace("Horse", defaultAttributes);
            sceneAnimalMap["Horse"].aA.SetAnimalType(AnimalAttributes::AnimalType::HOESE);
            sceneAnimalMap.emplace("Owl", defaultAttributes);
            sceneAnimalMap["Owl"].aA.SetAnimalType(AnimalAttributes::AnimalType::OWL);
            sceneAnimalMap["Owl"].aA.hungerDecayRate = 1.5; //owl special hunger
        }
    }

    void GameManager::ReviveAll() {
        deadAnimals = 0;

        const std::map<int, LevelConfig::LevelData>& lvlDataMap = levelConfig.GetLevels();

        for (std::pair<const std::string, AnimalData>& sceneAnimalPair : sceneAnimalMap) {
            if (!sceneAnimalPair.second.aA.is_alive && !sceneAnimalPair.second.aA.isEscaped)continue;
            if (lvlDataMap.empty() || lvlDataMap.find(levelNum) == lvlDataMap.end()) {
                AnimalAttributes defaultAttributes{};
                defaultAttributes.SetDefault();
                sceneAnimalPair.second.aA = defaultAttributes;
            }
            else {
                const LevelConfig::LevelData &lvlDat = lvlDataMap.at(levelNum);
                int animalNum{ sceneAnimalPair.second.aA.WhatAnimalType() };
                if (lvlDat.find(static_cast<LevelConfig::AnimalRef>(animalNum))==lvlDat.end())
                {
                    AnimalAttributes defaultAttributes{};
                    defaultAttributes.SetDefault();
                    sceneAnimalPair.second.aA = defaultAttributes;
                }
                else {
                    sceneAnimalPair.second.aA = lvlDat.at(static_cast<LevelConfig::AnimalRef>(animalNum));
                }
            }
        }

    }
    void GameManager::CheckAndDecreaseStats(float& time, float& chance, double decayRate, double& attributeValue, float frametime, double decreaseScale) {
        if (decayRate <= 0.0)decayRate = 0.00000001;
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

    bool LevelConfig::SetupLevelData(const std::string& folderPath) {
        if (!std::filesystem::exists(folderPath)||!std::filesystem::is_directory(folderPath))return false;
        
        std::filesystem::directory_iterator folder{ folderPath };
        for (const std::filesystem::directory_entry& file:folder)
        {
            std::filesystem::path filePath{ file.path() };
            if (!filePath.has_extension())continue;

            std::string ext = filePath.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            if (ext == ".json") {
                DeserializeFromFile(filePath.string());
            }
        }
        return true;
    }
    bool LevelConfig::Deserialize(const rapidjson::Value& obj) {
        if (!obj.HasMember("LevelIndex") || !obj["LevelIndex"].IsInt())return false;

        int levelIndex = obj["LevelIndex"].GetInt();

        if (levelList.find(levelIndex) != levelList.end())return false; //if u can find a level with that value then skip
        LevelData leveldat{};

        if (obj.HasMember("LevelData")&&obj["LevelData"].IsArray())
        {

             const rapidjson::Value::ConstArray arr = obj["LevelData"].GetArray();

             for (const rapidjson::Value& animal:arr)
             {
                 if (!animal.HasMember("Animal")||(!animal["Animal"].IsInt()&&!animal["Animal"].IsString())) continue;
                 AnimalRef aR{};
                 if (animal["Animal"].IsInt())aR = static_cast<AnimalRef>(animal["Animal"].GetInt());
                 if (animal["Animal"].IsString()) {
                     std::string animaltype= animal["Animal"].GetString();
                     std::transform(animaltype.begin(), animaltype.end(), animaltype.begin(), ::tolower);
                     if (animaltype == "flamingo")aR = FLAMINGO; 
                     if (animaltype == "seal")aR = SEAL;
                     if (animaltype == "horse")aR = HORSE;
                     if (animaltype == "owl")aR = OWL;
                 }

                 if (aR == IDK) continue;

                 if (!animal.HasMember("Attributes") || !animal["Attributes"].IsObject())continue;
                 AnimalAttributes aA{};
                 if (!aA.Deserialize(animal["Attributes"])) continue;
                 else leveldat.emplace(aR, aA);
                 //std::cout << "levelData emplaced\n";
             }
        }

        ExtraSceneSet extraScenes{};
        if (obj.HasMember("ExtraScenes") && obj["ExtraScenes"].IsArray())
        {
            for (const rapidjson::Value& scene : obj["ExtraScenes"].GetArray())
            {
                if (!scene.IsString())
                    continue;

                std::string sceneName = scene.GetString();
                std::transform(sceneName.begin(), sceneName.end(), sceneName.begin(), ::tolower);
                extraScenes.insert(sceneName);
            }
        }

        for (int i = 1; i < 5; i++)
        {
            if (leveldat.find(static_cast<AnimalRef>(i)) != leveldat.end())continue;
            AnimalAttributes a{};
            a.SetDefault();
            a.SetAnimalType(i - 1);
            leveldat.emplace(static_cast<AnimalRef>(i), a);
        }

        levelList.emplace(levelIndex, leveldat);
        extraScenesByLevel[levelIndex] = std::move(extraScenes);
        return true; 
    }

    bool LevelConfig::HasExtraScene(int levelIndex, const std::string& sceneName) const
    {
        auto levelIt = extraScenesByLevel.find(levelIndex);
        if (levelIt == extraScenesByLevel.end())
            return false;

        std::string normalizedName = sceneName;
        std::transform(normalizedName.begin(), normalizedName.end(), normalizedName.begin(), ::tolower);
        return levelIt->second.find(normalizedName) != levelIt->second.end();
    }

    //Pause and resume functions, also a function to check if the game is paused
    void GameManager::PauseGame() {
        if (currState == GameState::PAUSED) return;
        prePauseState = currState;
        currState = GameState::PAUSED;
        Graphic::layerEnabled[9] = true;   //show pause layer
        p_GUI->set_pause(true);
    }

    void GameManager::ResumeGame() {
        if (currState != GameState::PAUSED) return;
        currState = prePauseState;
        Graphic::layerEnabled[9] = false;  //hide pause layer
        p_GUI->set_pause(false);
    }

    bool GameManager::IsPaused() const {
        return currState == GameState::PAUSED;
    }
    void GameManager::SaveAnimalAttributesToScene(const std::string& sceneName, AnimalData aA) {
        if (currState == GameManager::GameState::ENDING) return;
        auto it = sceneAnimalMap.find(sceneName);
        if (it == sceneAnimalMap.end())
            sceneAnimalMap.emplace(sceneName, aA);
        else
            it->second = aA;
    }
}
