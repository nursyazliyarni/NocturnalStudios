/*!------------------------------------------------------------------------------------------------
	File: Application.cpp
	Project: NocturnalStudio

	This file contains the main application loop that initializes the window creation, OpenGL initialization,
	sprite rendering, input handling, drag and drop interations, frame updates, ImGui systems.

	Primary Author:

		- Brief

	Primary Author:
		Rishika Bimal Attodi
		- Implemented main loop scaffolding: window creation, OpenGL setup,
		sprite batching

	Secondary Author:
		Yu Xuan
		-  Background

	Secondary Authod
		Xiang Yang
		- FBO

	Secondary Author:
		Chang Qi Xuan Celest (35%)
		- Integrated drag and drop functionality for sprites.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "core/Application.h"


extern void Prefab_SetTextureLookup(std::function<int(const std::string&)>);

namespace {
	constexpr float TWO_PI = 2.0f * glm::pi<float>();
	constexpr int   CIRCLE_SEGMENTS = 64;
	using Clock = std::chrono::high_resolution_clock;

	bool isDragging = false; //is a sprite being dragged
	int draggedSpriteIndex = -1; //index into sprite_vector
	int duplicateIndex = -1; //where clone is in the vector
	Vector2D dragOffset(0.0f, 0.0f); //mouse offset from top-left of sprite
}

//Application::Application() = default;
Application::Application() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //enable memory leak check at program exit
}


void Application::init() {
	logger.setup_crash_handler();
	const GameConfig c = loadConfig("assets/config.json");
	windowTitle = c.window.title;

    WindowDesc wd;
    wd.width = c.window.width;
    wd.height = c.window.height;
    wd.title = c.window.title;
    wd.vsync = c.window.vsync;

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
	std::string vertexPath,
		fragmentPath,
		prefabPath, prefabTestPath,
		textShaderVertexPath, textShaderFragmentPath,
		scenePath;

	auto shaderPath_1 = resourceManager->getFilePathsForKey<ShaderFile>("background");
	for (auto const& path : shaderPath_1) {
		std::filesystem::path p{ path };
		auto ext = p.extension().string();
		if (ext == ".vert") vertexPath = p.string();
		else if (ext == ".frag") fragmentPath = p.string();
	}
	shader_filePATH_list.push_back({ vertexPath, fragmentPath });

	auto shaderPath_2 = resourceManager->getFilePathsForKey<ShaderFile>("special_background");
	for (auto const& path : shaderPath_2) {
		std::filesystem::path p{ path };
		auto ext = p.extension().string();
		if (ext == ".vert") vertexPath = p.string();
		else if (ext == ".frag") fragmentPath = p.string();
	}
	shader_filePATH_list.push_back({ vertexPath, fragmentPath });

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
	renderer = std::make_unique<Renderer>(shader_filePATH_list);
	textShader = std::make_unique<Shader>(textShaderVertexPath.c_str(), textShaderFragmentPath.c_str());
	logic = std::make_unique<Animal_Logic>();

    // After textures is constructed and before any Instantiate()
    Prefab_SetTextureLookup([this](const std::string& p) -> int {
        std::string path = p;
        if (path.rfind("assets/", 0) != 0) path = "assets/" + path;
        return static_cast<int>(textures.get(path));
    });

	prefabManager = std::make_unique<PrefabManager>(*ecs);
	prefabManager->Load_Prefab(prefabTestPath.c_str());
	prefabManager->CreateEntityFromPrefab(prefabTestPath, "main_menu_button", "play_button");
	prefabManager->CreateEntityFromPrefab(prefabTestPath, "main_menu_button", "load_button");
	prefabManager->CreateEntityFromPrefab(prefabTestPath, "main_menu_button", "setting_button");
	prefabManager->CreateEntityFromPrefab(prefabTestPath, "main_menu_button", "quit_button");
	prefabManager->CreateEntityFromPrefab(prefabTestPath, "food", "owl_food");
	prefabManager->CreateEntityFromPrefab(prefabTestPath, "animal", "flamingo");

	
	//logger.log("Created animal entity with ID: " + std::to_string(animal) + " and " + std::to_string(animal_2));
	std::cout << "abcssss:" << ecs->getAllEntities().size() << "\n";
    textShader = std::make_unique<Shader>(textShaderVertexPath.c_str(), textShaderFragmentPath.c_str());

    // Wire text shader into the renderer (renderer controls projection per-viewport)
    renderer->setTextShader(*textShader);

	imGuiManager = std::make_unique<ImGuiManager>(
		m_window, *renderer,
		*inputSystem, physicsSystem,
		*audioSystem, *resourceManager,
		*gsm, *logic);

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
        gameLoop->display_FPS(m_window.handle(), windowTitle, dt);
        inputSystem->update_mouse_pos(m_window.handle());

        for (int k = GLFW_KEY_1; k <= GLFW_KEY_9; ++k)
            if (inputSystem->WasPressed(k))
                gsm->switchToSceneForKey(k); // respects AppState::Playing

        if (inputSystem->WasPressed(GLFW_KEY_ESCAPE))
            gsm->handleKeyPress(GLFW_KEY_ESCAPE);

        inputSystem->ProcessHotkeys(*gsm);

        int fbw = 0, fbh = 0; glfwGetFramebufferSize(m_window.handle(), &fbw, &fbh);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, fbw, fbh);
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.06f, 0.06f, 0.07f, 1.0f);

        Prefab_SetTextureLookup([this](const std::string& p) -> int {
            return static_cast<int>(textures.get(p));
        });

        glClear(GL_COLOR_BUFFER_BIT);

        Gui::updateImGuiManager(dt, *imGuiManager);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, fbw, fbh);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Text overlay is now rendered by Renderer inside the viewport FBO (see ImGuiManager)

        if (inputSystem->WasMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
            ImVec2 rmin, rmax;

            imGuiManager->getViewportRectWindow(rmin, rmax);

            Vector2D mp = inputSystem->GET_mouse_pos(false, 0, 0);
            if (mp.x() >= rmin.x && mp.x() <= rmax.x &&
                mp.y() >= rmin.y && mp.y() <= rmax.y) {
                float nx = float((mp.x() - rmin.x) / (rmax.x - rmin.x));
                float ny = float((mp.y() - rmin.y) / (rmax.y - rmin.y));

                ny = 1.0f - ny;
                gsm->handleClick(nx, ny);
            }
        }
		TransformSystem::TransformUpdate();
        if (gsm->popQuitRequested()) {
            glfwSetWindowShouldClose(m_window.handle(), 1);
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
