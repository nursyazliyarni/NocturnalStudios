/*!------------------------------------------------------------------------------------------------
    File: Application.h
    Project: NocturnalStudio

    This file contains Application class to start the application defined in Application.cpp.

    Rishika Bimal Attodi
        - Declared Application interface and main run entrypoint.
        - stabilized init & main loop, wired systems, 
        fixed menu menu to scene transitions, and cleaned up redundant code paths.

    Secondary Author:
         Lau Xiang Yang (20%)
         - Implemented onFocusChanged and onIconifyChanged methods to handle window focus and iconify events.
         - Implemented auto-minimization and auto-resumption of the application based on window focus state.
        

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

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
    /**
     * @brief Construct an Application object.
     *
     * Does minimal construction work. Heavy initialization (window, renderer,
     * resource loading, subsystems) is performed in init().
     */
    Application();

    /**
     * @brief Initialize the application and subsystems.
     *
     * - Sets up crash handling via the global logger.
     * - Loads configuration from "assets/config.json".
     * - Creates the GLFW window and OpenGL context.
     * - Initializes input, ECS, ResourceManager, AudioSystem and other core systems.
     * - Loads resources (audio, shaders, textures, fonts, prefabs, scenes).
     * - Creates renderer, text shader, prefab manager, game manager and ImGui manager.
     *
     * After a successful init(), Application::run() may be called. If initialization
     * fails the isInitialized flag remains false.
     */
    void init();

    /**
     * @brief Main application loop entrypoint.
     *
     * Calls init() if necessary, then enters the primary loop which:
     * - Polls window events.
     * - Advances the game loop and computes dt.
     * - Updates GameManager, systems and ImGui.
     * - Handles input, hotkeys and scene switching.
     * - Renders the frame and swaps buffers.
     *
     * Returns 0 on normal exit, or a negative error code if initialization failed.
     */
    int run();

    /**
     * @brief Shutdown and clean up application subsystems and resources.
     *
     * - Releases resources held by ResourceManager and other singletons.
     * - Destroys ImGui, renderer, audio, resource/prefab managers, game loop and input.
     * - Destroys the GLFW window and marks the application uninitialized.
     */
    void shutdown();

    /**
     * @brief Handle change in window focus state.
     *
     * Called from the GLFW focus callback. When the window loses focus the
     * method will optionally iconify the window (autoMinimizeOnFocusLoss),
     * pause audio (BGM & SFX) and pause the in-game state via GameManager.
     *
     * When focus is regained the method optionally restores audio and game
     * state (autoResumeOnFocusGain) and performs end-of-frame input housekeeping.
     *
     * @param focused true if window gained focus, false if focus was lost.
     */
    void onFocusChanged(bool focused);

    /**
     * @brief Handle window iconify (minimize / restore) events.
     *
     * Called from the GLFW iconify callback. When iconified the method pauses
     * audio and the game manager and clears input state. When restored it will
     * optionally resume audio and the game manager, and perform input end-frame.
     *
     * @param iconified true when the window was iconified (minimized), false when restored.
     */
    void onIconifyChanged(bool iconified);

    /**
     * @brief Handle external file drop (GLFW drop callback)
     *
     * Called when files are dragged and dropped onto the window.
     * This can be used to load assets like scenes, textures or fonts at runtime.
     *
     * @param count The number of dropped files.
     * @param paths The array of file paths (UTF-8 encoded strings).
     */
    void onFilesDropped(int count, const char** paths);

private: // window and context
    Window m_window;
    bool imGUIVisible{ false };
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
