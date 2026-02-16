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

#include "Core/Game.h"
#include "System/ServiceLocator.h"
#include "LevelEditor/LevelEditor.h"
#include "GUI/GUI.h"
namespace NocturnalEngine {
    
    void NocturnalGameLayer::Init() {
        m_gameConfig = LuminaraZoo();
        ServiceLocator::Get().ProvideConfig(&m_gameConfig);
    }
    void NocturnalGameLayer::Update(float dt) {
        bool isHowToPlayScene = false;
        for (Entity entity = 0; entity < p_ecs.GetTotalEntities(); ++entity)
        {
            if (!p_ecs.HaveComponent<Tag>(entity))
            {
                continue;
            }

            if (p_ecs.GetComponent<Tag>(entity).GetTag() != "HowToPlay")
            {
                continue;
            }

            if (p_ecs.HaveComponent<NameTag>(entity))
            {
                const std::string& name = p_ecs.GetComponent<NameTag>(entity).GetNameTag();
                if (name == "HTPBackground" || name == "Background")
                {
                    isHowToPlayScene = true;
                    break;
                }
            }

            if (p_Editor->is_ShowWindow == false &&
                p_Input->KeyPressed(GLFW_KEY_ESCAPE) &&
                (p_Scene->isGameplay || p_GUI->check_pause() || isHowToPlayScene))
            {
                const bool shouldResetGameplayState = (p_Scene->isGameplay || p_GUI->check_pause());
                if (shouldResetGameplayState)
                {
                    p_GameManager.ResetSystem(false);
                }

                if (p_GUI->check_pause())
                {
                    p_GUI->toggle_pause();
                }
                p_GUI->sub_menu = false;
                p_Scene->setSceneToLoad("Assets/Scene/Menu.json");

                if (p_Input->KeyPressed(GLFW_KEY_F3))
                {
                    for (Entity i = 0; i < p_ecs.GetTotalEntities(); ++i)
                    {


                        if (p_ecs.HaveComponent<Logic>(i) && p_ecs.GetComponent<Logic>(i).GetScriptByName("DialogueManager") != nullptr)
                        {
                            p_ecs.GetComponent<Logic>(i).GetScriptByName("DialogueManager")->mScriptPause = true;
                        }

                    }
                }
                for (Entity entity = 0; entity < p_ecs.GetTotalEntities(); entity++)
                {
                    if (p_ecs.HaveComponent<NameTag>(entity) && p_ecs.GetComponent<NameTag>(entity).GetNameTag() == "Splash Screen")
                    {
                        if (p_ecs.GetComponent<Transform>(entity).GetScale().x > 3.5f)
                            p_Scene->setSceneToLoad("Assets/Scene/Menu.json");
                    }
                }
            }
        }
    }
    void NocturnalGameLayer::Shutdown() {
        
    }

    void NocturnalGameLayer::RegisterScripts() {
        // Register all scripts
        m_scriptFactory.RegisterAllGameScripts();

        // Provide to engine
        ServiceLocator::Get().ProvideScriptRegistry(&m_scriptFactory);


    }
    
}