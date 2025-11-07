/*!------------------------------------------------------------------------------------------------
    File: Application.cpp
    Project: NocturnalStudio

    This file contains the main application loop that initializes the window creation, OpenGL
    initialization, sprite rendering, input handling, drag and drop interactions, frame updates,
    and ImGui systems.

    Primary Author:
        Rishika Bimal Attodi
        - Implemented main loop scaffolding: window creation, OpenGL setup, sprite batching

    Secondary Author:
        Yu Xuan
        - Background

    Secondary Author
        Xiang Yang
        - FBO

    Secondary Author:
        Chang Qi Xuan Celest (35%)
        - Integrated drag and drop functionality for sprites.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "core/Application.h"

#include <functional>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <cstdlib>
#include <ctime>

#include "scene/Prefab.h"             // Prefab_SetTextureLookup, Scenes_GetInitialName, Prefab_SpawnEntitiesForScene
#include "ECS/ECS.h"
#include "ECS/ECS_Component.h"
#include "ECS/ECS_DEF.h"
#include "resources/ResourceManager.h"

extern void Prefab_SetTextureLookup(std::function<int(const std::string&)>);
extern Logger logger;

//  simple AI drift (requires AIComponent + PhysicsComponent to exist)
static void UpdateRandomDrift(ECS& ecs, double dt) {
    if (dt <= 0.0) return;

    static bool seeded = false;
    if (!seeded) { std::srand(static_cast<unsigned>(std::time(nullptr))); seeded = true; }

    struct DriftState { float heading = 0.0f; float nextTurn = 0.0f; };
    static std::unordered_map<EntityID, DriftState> s_state;

    auto frand = [](float a, float b) -> float {
        return a + (b - a) * (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX));
    };

    constexpr EntityID MAX_SCAN = 5000;

    for (EntityID e = 0; e < MAX_SCAN; ++e) {
        auto* ai = ecs.getComponent<AIComponent>(e);
        auto* pc = ecs.getComponent<PhysicsComponent>(e);
        if (!ai || !pc) continue;

        auto* t = ecs.getComponent<Transform>(e);
        if (!t) { Transform nt{}; ecs.addComponent<Transform>(e, nt); t = ecs.getComponent<Transform>(e); if (!t) continue; }

        DriftState& st = s_state[e];
        if (st.nextTurn <= 0.0f && st.heading == 0.0f) {
            st.heading = frand(0.0f, 6.2831853f);
            st.nextTurn = frand(0.6f, 1.8f);
        }

        st.nextTurn -= static_cast<float>(dt);
        if (st.nextTurn <= 0.0f) {
            st.heading += frand(-0.9f, 0.9f);
            if (st.heading > 6.2831853f) st.heading -= 6.2831853f;
            if (st.heading < 0.0f)       st.heading += 6.2831853f;
            st.nextTurn = frand(0.6f, 1.8f);
        }

        float desiredVx = std::cos(st.heading) * ai->maxSpeed;
        float desiredVy = std::sin(st.heading) * ai->maxSpeed;

        float ax = desiredVx - pc->velocity.x();
        float ay = desiredVy - pc->velocity.y();
        float aLen2 = ax * ax + ay * ay;
        float maxA = ai->accel * static_cast<float>(dt);
        if (aLen2 > maxA * maxA && aLen2 > 0.0f) {
            float s = maxA / std::sqrt(aLen2);
            ax *= s; ay *= s;
        }
        pc->velocity.x() += ax;
        pc->velocity.y() += ay;

        float v2 = pc->velocity.x() * pc->velocity.x() + pc->velocity.y() * pc->velocity.y();
        float ms = ai->maxSpeed;
        if (v2 > ms * ms && v2 > 0.0f) {
            float k = ms / std::sqrt(v2);
            pc->velocity.x() *= k; pc->velocity.y() *= k;
        }

        float nx = t->position.x() + pc->velocity.x() * static_cast<float>(dt);
        float ny = t->position.y() + pc->velocity.y() * static_cast<float>(dt);
        bool bounceX = false, bounceY = false;

        if (nx < 0.0f) { nx = 0.0f; bounceX = true; }
        if (nx > 1.0f) { nx = 1.0f; bounceX = true; }
        if (ny < 0.0f) { ny = 0.0f; bounceY = true; }
        if (ny > 1.0f) { ny = 1.0f; bounceY = true; }

        if (bounceX) {
            pc->velocity.x() = -pc->velocity.x();
            st.heading = std::atan2(pc->velocity.y(), pc->velocity.x());
        }
        if (bounceY) {
            pc->velocity.y() = -pc->velocity.y();
            st.heading = std::atan2(pc->velocity.y(), pc->velocity.x());
        }

        t->position.x() = nx;
        t->position.y() = ny;
    }
}

// One anonymous namespace only (no duplicates)
namespace {
    constexpr float TWO_PI = 2.0f * glm::pi<float>();
    constexpr int   CIRCLE_SEGMENTS = 64;
    using Clock = std::chrono::high_resolution_clock;

    bool isDragging = false;
    int  draggedSpriteIndex = -1;
    int  duplicateIndex = -1;
    Vector2D dragOffset(0.0f, 0.0f);
}

// ctor
Application::Application() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}

void Application::init() {
    logger.setup_crash_handler();
    const GameConfig c = loadConfig("assets/config.json");
    windowTitle = c.window.title;

    WindowDesc wd;
    wd.width = c.window.width;
    wd.height = c.window.height;
    wd.title  = c.window.title;
    wd.vsync  = c.window.vsync;

    if (!m_window.create(wd)) {
        logger.error("Failed to create window");
        isInitialized = false;
        return;
    }

    glViewport(0, 0, wd.width, wd.height);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    m_window.swap();

    int currW = wd.width, currH = wd.height;
    glfwGetFramebufferSize(m_window.handle(), &currW, &currH);
    glViewport(0, 0, currW, currH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, currW, currH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    inputSystem = std::make_unique<Input>(m_window.handle());
    inputSystem->SetMessageBus(&bus);

    ecs = &ECS::getECS();
    resourceManager = std::make_unique<ResourceManager>(*ecs);
    audioSystem = std::make_unique<AudioSystem>("AudioSystem", *ecs, *resourceManager);
    resourceManager->setAudioSystem(audioSystem.get());

    resourceManager->loadResources("audio");
    resourceManager->loadResources("shaders");
    resourceManager->loadResources("textures");
    resourceManager->loadResources("fonts");
    resourceManager->loadResources("prefabs");
    resourceManager->loadResources("scenes");

    std::vector<std::pair<std::string, std::string>> shader_filePATH_list;
    std::string vertexPath, fragmentPath, prefabPath, prefabTestPath, textShaderVertexPath, textShaderFragmentPath, scenePath;

    // background shader (pair: vert/frag)
    auto shaderPath_1 = resourceManager->getFilePathsForKey<ShaderFile>("background");
    for (auto const& path : shaderPath_1) {
        std::filesystem::path p{ path };
        auto ext = p.extension().string();
        if (ext == ".vert") vertexPath = p.string();
        else if (ext == ".frag") fragmentPath = p.string();
    }
    shader_filePATH_list.push_back({ vertexPath, fragmentPath });

    // special background shader (pair: vert/frag)
    auto shaderPath_2 = resourceManager->getFilePathsForKey<ShaderFile>("special_background");
    for (auto const& path : shaderPath_2) {
        std::filesystem::path p{ path };
        auto ext = p.extension().string();
        if (ext == ".vert") vertexPath = p.string();
        else if (ext == ".frag") fragmentPath = p.string();
    }
    shader_filePATH_list.push_back({ vertexPath, fragmentPath });

    // text shader (pair: vert/frag)
    auto textShaderPaths = resourceManager->getFilePathsForKey<ShaderFile>("text");
    for (auto const& path : textShaderPaths) {
        std::filesystem::path p{ path };
        auto ext = p.extension().string();
        if (ext == ".vert") textShaderVertexPath = p.string();
        else if (ext == ".frag") textShaderFragmentPath = p.string();
    }

    auto prefabPaths = resourceManager->getFilePathsForKey<PrefabFile>("prefabs");
    if (!prefabPaths.empty()) prefabPath = prefabPaths.front();

    auto testingPrefabPaths = resourceManager->getFilePathsForKey<PrefabFile>("testing_prefabs");
    if (!testingPrefabPaths.empty()) prefabTestPath = testingPrefabPaths.front();

    auto scenePaths = resourceManager->getFilePathsForKey<SceneFile>("scenes");
    if (!scenePaths.empty()) scenePath = scenePaths.front();

    gameLoop = std::make_unique<Game_Loop>(m_window.handle());
    bus.Register(gameLoop.get());

    gsm = std::make_unique<GameManager>(*ecs);
    gsm->load(prefabPath.c_str(), scenePath.c_str(), textures);

    // One quit callback only
    gsm->quitCallback = [this]() {
        logger.log("[Quit] quitCallback → glfwSetWindowShouldClose(TRUE)");
        glfwSetWindowShouldClose(m_window.handle(), GLFW_TRUE);
    };

    // Renderer (shader list variant)
    renderer  = std::make_unique<Renderer>(shader_filePATH_list);
    textShader = std::make_unique<Shader>(textShaderVertexPath.c_str(), textShaderFragmentPath.c_str());
    logic     = std::make_unique<Animal_Logic>();

    // Texture lookup for prefab sprite paths (robust: add "assets/" if missing)
    Prefab_SetTextureLookup([this](const std::string& p) -> int {
        std::string path = p;
        if (path.rfind("assets/", 0) != 0) path = "assets/" + path;
        return static_cast<int>(textures.get(path));
    });

    prefabManager = std::make_unique<PrefabManager>(*ecs);

    // Load every prefab file discovered by ResourceManager
    for (auto& [pid, pf] : resourceManager->getPrefabs()) {
        prefabManager->Load_Prefab(pf.getFilePath());
    }
    logger.log("[Prefabs] Loaded " + std::to_string(resourceManager->getPrefabs().size()) + " prefab file(s).");

    std::cout << "abcssss:" << ecs->getAllEntities().size() << "\n";

    // Text shader into renderer
    renderer->setTextShader(*textShader);

    // Spawn initial scene (from scenes.json -> states.initial)
    {
        const std::string initialScene = Scenes_GetInitialName();
        if (!initialScene.empty()) {
            Prefab_SpawnEntitiesForScene(initialScene, *prefabManager);
            logger.log("[SceneSpawn] Initial scene spawned: " + initialScene);
        }
    }

    // On scene change, wipe + respawn from JSON
    gsm->setSceneChangedCallback([this](const std::string& sceneName) {
        auto all = ecs->getAllEntities();
        for (auto id : all) ecs->deleteEntity(id);

        Prefab_SpawnEntitiesForScene(sceneName, *prefabManager);
        logger.log(std::string{ "[SceneSpawn] Activated -> " } + sceneName);
    });

    imGuiManager = std::make_unique<ImGuiManager>(
        m_window, *renderer,
        *inputSystem, physicsSystem,
        *audioSystem, *resourceManager,
        *gsm, *logic
    );

    isInitialized = true;
}

int Application::run() {
    if (!isInitialized) {
        init();
    }

    if (!isInitialized) {
        logger.error("Application not initialized. Exiting run().");
        return -1;
    }

    while (!m_window.shouldClose()) {
        m_window.poll();

        double dt = gameLoop->calculate_DT();
        gsm->update(static_cast<float>(dt), *renderer, textures);
        Gui::updateImGuiManager(dt, *imGuiManager);
        gameLoop->display_FPS(m_window.handle(), windowTitle, dt);
        inputSystem->update_mouse_pos(m_window.handle());

        //  simple AI drift (safe no-op if components absent)
        if (ecs) UpdateRandomDrift(*ecs, dt);

        // Number keys 1..9 (respects AppState::Playing inside GameManager)
        for (int k = GLFW_KEY_1; k <= GLFW_KEY_9; ++k)
            if (inputSystem->WasPressed(k))
                gsm->switchToSceneForKey(k);

        if (inputSystem->WasPressed(GLFW_KEY_ESCAPE))
            gsm->handleKeyPress(GLFW_KEY_ESCAPE);

        inputSystem->ProcessHotkeys(*gsm);

        int fbw = 0, fbh = 0; glfwGetFramebufferSize(m_window.handle(), &fbw, &fbh);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, fbw, fbh);
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, fbw, fbh);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Mouse click → normalized coords within viewport (flip Y to match your JSON/UI math)
        if (inputSystem->WasMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
            ImVec2 rmin, rmax;
            imGuiManager->getViewportRectWindow(rmin, rmax);

            Vector2D mp = inputSystem->GET_mouse_pos(false, 0, 0);
            if (mp.x() >= rmin.x && mp.x() <= rmax.x &&
                mp.y() >= rmin.y && mp.y() <= rmax.y) {

                float nx = float((mp.x() - rmin.x) / (rmax.x - rmin.x));
                float ny = float((mp.y() - rmin.y) / (rmax.y - rmin.y));
                ny = 1.0f - ny; // keep this flip (your earlier working path)

                logger.log("[Click] nx=" + std::to_string(nx) + " ny=" + std::to_string(ny) + " (viewport)");
                gsm->handleClick(nx, ny);

                if (gsm->popQuitRequested()) {
                    if (gsm->quitCallback) gsm->quitCallback();
                    else glfwSetWindowShouldClose(m_window.handle(), GLFW_TRUE);
                    break; // leave the loop now
                }
            }
        }

        inputSystem->EndFrame();
        m_window.swap();
    }

    shutdown();
    return 0;
}

void Application::shutdown() {
    if (resourceManager) {
        for (auto& [id, font] : resourceManager->getFonts()) {
            font.Cleanup();
        }
    }

    imGuiManager.reset();
    textShader.reset();
    renderer.reset();
    audioSystem.reset();
    resourceManager.reset();
    prefabManager.reset();
    gsm.reset();
    gameLoop.reset();
    inputSystem.reset();

    m_window.destroy();
    isInitialized = false;
}
