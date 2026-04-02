/*!------------------------------------------------------------------------------------------------
    File: Game.cpp
    Project: NocturnalStudio

    Brief:
        This file contains the declaration of Game class. This is any logic that the game will run on 

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the Game class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "Core/Game.h"
#include "System/ServiceLocator.h"
#include "LevelEditor/LevelEditor.h"
#include "GUI/GUI.h"
#include "ECS/SceneManager.h"
#include "ECS/GameComponentReg.h"
#include "ECS/PrefabManager.h"
namespace NocturnalEngine {
    
    void NocturnalGameLayer::Init() {
        m_gameConfig = LuminaraZoo();
        ServiceLocator::Get().ProvideConfig(&m_gameConfig);

        GameComponentRegistry::GetInstance()->RegisterAllGameComponents();
        ServiceLocator::Get().ProvideGameComponents(GameComponentRegistry::GetInstance());

        p_GameManager.Init();

#if !DEBUG
        p_Scene->setSceneToLoad("Assets/Scene/Menu.json");
#endif
    }
    void NocturnalGameLayer::Update(float dt) {
        
        bool isHowToPlayScene = false;
        for (Entity entity = 0; entity < p_ecs.GetTotalSceneEntities(); ++entity)
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
        }
        /*if (p_Editor->is_ShowWindow == false &&
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
        }*/

        bool escDown = p_Input->KeyPressed(GLFW_KEY_ESCAPE);
        bool escTriggered = escDown && !escPressedLastFrame;
        escPressedLastFrame = escDown;

        if (p_Editor->is_ShowWindow == false &&
            escTriggered &&
            (p_Scene->isGameplay || p_GUI->check_pause() || isHowToPlayScene))
        {
            if (isHowToPlayScene) {
                p_Scene->setSceneToLoad("Assets/Scene/Menu.json");
                return;
            }

            if (p_Scene->isGameplay) {
                if (p_GameManager.IsPaused()) {
                    p_GameManager.ResumeGame();
                }
                else {
                    p_GameManager.PauseGame();
                }
            }
        }

        p_GameManager.Update(dt);

        if (p_Input->KeyPressed(GLFW_KEY_F3))
        {
            for (Entity i = 0; i < p_ecs.GetTotalSceneEntities(); ++i)
            {


                if (p_ecs.HaveComponent<Logic>(i) && p_ecs.GetComponent<Logic>(i).GetScriptByName("DialogueManager") != nullptr)
                {
                    p_ecs.GetComponent<Logic>(i).GetScriptByName("DialogueManager")->mScriptPause = true;
                }

            }
        }
        for (Entity entity = 0; entity < p_ecs.GetTotalSceneEntities(); entity++)
        {
            if (p_ecs.HaveComponent<NameTag>(entity) && p_ecs.GetComponent<NameTag>(entity).GetNameTag() == "Splash Screen")
            {
                if (p_ecs.GetComponent<Transform>(entity).GetScale().x > 3.5f)
                    p_Scene->setSceneToLoad("Assets/Scene/Menu.json");
            }
        }
           
        
    }
    void NocturnalGameLayer::Shutdown() {
        
    }
    void NocturnalGameLayer::InputUpdate(float dt) {
        if (!m_window)return;
        if (p_Editor->is_ShowWindow == false) {

            if (p_Input->GamepadGotConnected())
            {
                if (p_PrefabManager.CheckOrAddPrefabFromName("GamepadConnected")) {
                    p_PrefabManager.CloneFromPrefab("GamepadConnected");
                }
            }
            else if (p_Input->GamepadGotDisconnected()) {
                if (p_PrefabManager.CheckOrAddPrefabFromName("GamepadDisconnected")) {
                    p_PrefabManager.CloneFromPrefab("GamepadDisconnected");
                }
            }

            if (p_Input->IsGamepadConnected() && p_Input->GetLastDevice() == InputDevice::Gamepad)
            {
                static constexpr float CURSOR_SPEED = 800.0f; // pixels per second

                float stickX = p_Input->GetAxis(GLFW_GAMEPAD_AXIS_LEFT_X);
                float stickY = p_Input->GetAxis(GLFW_GAMEPAD_AXIS_LEFT_Y);

                if (std::abs(stickX) > 0.0f || std::abs(stickY) > 0.0f)
                {
                    WindowProps windowData = m_window->Getter();
                    // Read current cursor position
                    double curX = windowData.mouseX;
                    double curY = windowData.mouseY;

                    // Move it
                    curX += stickX * CURSOR_SPEED * dt;
                    curY += stickY * CURSOR_SPEED * dt;

                    // Clamp to window bounds
                    curX = std::clamp(curX, 0.0, (double)windowData.m_Width);
                    curY = std::clamp(curY, 0.0, (double)windowData.m_Height);

                    // Write back into your existing mouse position data
                    m_window->GetMouseX() = curX;
                    m_window->GetMouseY() = curY;

                    // Move the actual OS cursor to match (so your click detection stays in sync)
                    glfwSetCursorPos(m_window->GetWindow(), curX, curY);
                }

                // A button = left click
                if (p_Input->GamepadPressed(GLFW_GAMEPAD_BUTTON_A))
                    p_Input->SetMouseStatus(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
                else if (p_Input->GamepadReleased(GLFW_GAMEPAD_BUTTON_A))
                    p_Input->SetMouseStatus(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
            }
        }


    }
    void NocturnalGameLayer::RegisterScripts() {
        // Register all scripts
        m_scriptFactory.RegisterAllGameScripts();

        // Provide to engine
        ServiceLocator::Get().ProvideScriptRegistry(&m_scriptFactory);


    }
    
}