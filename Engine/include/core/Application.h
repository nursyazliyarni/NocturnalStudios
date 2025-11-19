/*!------------------------------------------------------------------------------------------------
    File: Application.h
    Project: NocturnalStudio

	This file contains Application class to start the application defined in Application.cpp.

    Rishika Bimal Attodi
        - Declared Application interface and main run entrypoint.
        - stabilized init & main loop, wired systems, 
        fixed menu menu to scene transitions, and cleaned up redundant code paths.

    Secondary Author:
         Name (%)
        -Brief

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// init(), run(), shutdown()

#pragma once
#include "graphics/Window.h"
#include "graphics/Texture.h"
#include "core/GameLoop.h"
#include "graphics/Shader.h"
#include "graphics/Renderer.h"
#include "physics/Physics.h"
#include "core/Logger.h"
#include "core/Config.h"
#include "math/Vector.h"
#include "core/ImGUIManager.h"
#include "physics/AABB.h"
#include "ECS/ECS.h"
#include "resources/ResourceManager.h"
#include "core/GameManager.h"
#include "scene/Prefab.h"
#include "./scene/SceneManager.h"
#include "resources/Font.h"
#include "graphics/Camera2D.h"
#include "logic/Animal_Logic.h"
#include "Animation/AnimationSystem.h"
#include "System/Transformation_System.h"

#include <GLFW/glfw3.h>            // glViewport/glClearColor/glClear
#include <cmath>                   // cosf, sinf
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>   // glm::pi<float>()
#include <chrono>

extern Logger logger;

class Application {
public:
    Application();
    void init();
    int run();            // create window from JSON + minimal loop
    void shutdown();

    void onFocusChanged(bool focused);
    void onIconifyChanged(bool iconified);

private: // window and context
    Window m_window;
    bool isInitialized{ false };
    std::string windowTitle;

    bool autoMinimizeOnFocusLoss{ true };
    bool autoResumeOnFocusGain{ true };

    private: // core subsystem and resources kept alive across run()
    ECS* ecs{ nullptr };
    Font* uiFont{ nullptr };
	Font* uiFont2{ nullptr };

    Messaging::MessageBus bus;
    TextureStore textures;
	Physics physicsSystem;

    std::unique_ptr<Game_Loop> gameLoop;
	std::unique_ptr<ResourceManager> resourceManager;
	std::unique_ptr<AudioSystem> audioSystem;
	std::unique_ptr<GameManager> gsm;
    std::unique_ptr<Animal_Logic> logic;
	std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Shader> textShader;
	std::unique_ptr<ImGuiManager> imGuiManager;
	std::unique_ptr<PrefabManager> prefabManager;
    std::unique_ptr<Input> inputSystem;
    std::unique_ptr<AnimationSystem> animationSystem;
};
