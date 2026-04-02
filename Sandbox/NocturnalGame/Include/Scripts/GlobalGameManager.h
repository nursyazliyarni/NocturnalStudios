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
//#include "Scripts/AnimalLogic.h"

#define p_GameManager NocturnalEngine::GameManager::GetInstance()

namespace NocturnalEngine {

    class LevelConfig:public JSONSerializer {
    public:
        enum AnimalRef
        {
            IDK = 0,
            FLAMINGO = 1,
            SEAL = 2,
            HORSE = 3,
            OWL = 4
        };

        
        typedef std::map<AnimalRef, AnimalAttributes> LevelData;
        
        
        bool SetupLevelData(const std::string& folderPath);

        virtual bool Deserialize(const rapidjson::Value& obj) override;

        virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const override { UNREFERENCED_PARAMETER(writer); return true; }

        const std::map<int,LevelData>& GetLevels() const { return levelList; }

    private:
        std::map<int,LevelData> levelList{};
    };


    class GameManager {
    public:

        struct ToyState
        {
            bool isHiding{ false };
            int hidingIndex{ 0 };
        };
        struct AnimalData
        {
            AnimalAttributes aA{};
            float  hungerD {10.f }, hpD{ 10.f }, sanityD{ 10.f };
            float  hungerT{ 0.f }, hpT{ 0.f }, sanityT{ 0.f };
            ToyState toy{ };
        };


        enum class GameState {
            STARTING=0,
            PLAYING=1,
            ENDING=2,
            PAUSED=3
        };
        AnimalData LoadPersistentValues(const std::string& gs);
        ToyState LoadToyState(const std::string& gs);

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

         void NextLevel();

         void SaveAnimalAttributesToCurrentScene(AnimalData aA);

         void SaveToyState(bool hiding, int hideIndex);

         GameState GetGameState()const { return currState; }

         void SetState(GameState s) { currState = s; }

         void Pause() { prePauseState = currState; currState = GameState::PAUSED; }

         void UnPause() { currState = prePauseState; }

         unsigned int CurrentLevel() const { return levelNum; }
        
         void AddDeath() { deadAnimals++; }

         void AddEscape() { escapedAnimals++; }

         void AddCrit() { under20++; }

         int AnimalsGone() const { return deadAnimals + escapedAnimals; }

         int getCrits() const{ return under20; }

         //returns how much we have to multiply seconds by for timer
         float GetTimeScale() const{ return 21600.f/winRequirement; }

         float GetCheckPeriod()const { return winRequirement/72.f; }

         float GetTimeElapsed() const{ return gameTimeElapsed; }

         bool HasLogBeenUpdated();

         const std::deque<std::string>& GetLogEntries()  { return diegeticLogEntries; }

         void AddLogEntry(std::string message);

         bool isTargetVisible()const { return targetVisibility; }

         void setTargetVisibility(bool t) { targetVisibility = t; }

         bool IsNewSceneManagement() const { return altMode; }

         void ToggleSceneManagement() { altMode = !altMode; }

         void SetSceneToSwitch(std::string);

         std::string CanMoveScene(const std::string&, bool dir);

         void AllFullStats();
         void SetTime(float time) { gameTimeElapsed = time; }
         void ReviveAll();

    private:

        LevelConfig levelConfig{};

        GameManager();
        ~GameManager();
        GameManager(const GameManager&) = delete;
        GameManager& operator=(const GameManager&) = delete;

        static GameManager instance;

        void LoseGame();
        void WinGame();
        void LogLowHealth(std::pair < const std ::string , AnimalData > & sceneAnimalPair);


        float gameTimeElapsed{}, winRequirement{120.f}; //in seconds

        std::string currScene;
        GameState currState, prePauseState;

        std::unordered_map < std::string, AnimalData> sceneAnimalMap;
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

        unsigned int levelNum{ 2 }, levelCount{ 3 };
        bool levelDataLoaded{ false };
        bool LevelDataToMap(int levelN);

        void CheckAndDecreaseStats(float& time, float& chance, double decayRate, double& attributeValue, float frametime, double decreaseScale=1.0);
        bool CheckHide(AnimalAttributes& aa);

    };


}