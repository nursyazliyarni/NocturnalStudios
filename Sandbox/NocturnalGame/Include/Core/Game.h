/*!------------------------------------------------------------------------------------------------
    File: Game.h
    Project: NocturnalStudio

    Brief:
        This file contains the declaration of EngineCore class.

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the EngineCore class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "Core/IGameLayer.h"
#include "core/IGameConfig.h"
#include "Scripts/GlobalGameManager.h"
#include "Factories/ScriptFactory.h"
namespace NocturnalEngine {
    class LuminaraZoo : public IGameConfig {
    public:
        LuminaraZoo() {
            // Define your playable scenes
            mPlayableScenes = {
                "FlamingoScene.json",
                "HorseScene.json",
                "SealScene.json",
                "OwlScene.json",
                "CombinedScene.json"
            };
            mainMenuScene = "Assets/Scene/Menu.json";
        }

        bool IsPlayableScene(const std::string& sceneName) const override {
            return mPlayableScenes.count(sceneName) > 0;
        }

        void OnEnterPlayMode(const std::string& currentScene) override {
            // Handle CombinedScene special case
            if (currentScene == "CombinedScene.json") {
                p_GameManager.ToggleSceneManagement();
            }
            else if (p_GameManager.IsNewSceneManagement()) {
                p_GameManager.ToggleSceneManagement();
            }

            p_GameManager.SetState(GameManager::GameState::PLAYING);
        }

        void OnExitPlayMode(bool isEditor) override {
            p_GameManager.ResetSystem(isEditor);
        }

    private:
        std::unordered_set<std::string> mPlayableScenes;
    };


    class NocturnalGameLayer : public IGameLayer
    {
        virtual void Init() override;
        virtual void Update(float dt) override;
        virtual void Shutdown();

        void RegisterScripts() override;
    private:
        ScriptFactory m_scriptFactory;
        LuminaraZoo m_gameConfig;
        bool escPressedLastFrame{ false };
    };
}


