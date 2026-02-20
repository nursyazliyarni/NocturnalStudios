/*!------------------------------------------------------------------------------------------------
    File: Game.h
    Project: NocturnalStudio

    Brief:
        This file contains the declaration of Game class.

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the EngineCore class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "Core/IGameLayer.h"
#include "core/IGameConfig.h"
#include "Factories/ScriptFactory.h"
namespace NocturnalEngine {
    class NewGame : public IGameConfig {
    public:
        NewGame() {
            // Define your playable scenes
            mPlayableScenes = {};
        }

        bool IsPlayableScene(const std::string& sceneName) const override {
            return mPlayableScenes.count(sceneName) > 0;
        }

        void OnEnterPlayMode(const std::string& currentScene) override {
            
        }

        void OnExitPlayMode(bool isEditor) override {
            
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
        NewGame m_gameConfig;
    };
}


