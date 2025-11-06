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

    std::string vertexPath,
        fragmentPath,
        prefabPath, prefabTestPath,
        textShaderVertexPath, textShaderFragmentPath,
        scenePath;

    auto shaderPath = resourceManager->getFilePathsForKey<ShaderFile>("background");
    for (auto const& path : shaderPath) {
        std::filesystem::path p{ path };
        auto ext = p.extension().string();
        if (ext == ".vert") vertexPath = p.string();
        else if (ext == ".frag") fragmentPath = p.string();
    }

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

    // After textures is constructed and before any Instantiate()
    Prefab_SetTextureLookup([this](const std::string& p) -> int {
        std::string path = p;
        if (path.rfind("assets/", 0) != 0) path = "assets/" + path;
        return static_cast<int>(textures.get(path));
    });

    renderer = std::make_unique<Renderer>(vertexPath.c_str(), fragmentPath.c_str());
    textShader = std::make_unique<Shader>(textShaderVertexPath.c_str(), textShaderFragmentPath.c_str());

    {
        ResourceID fontID = resourceManager->findResource<Font>("NanumBrushScript-Regular");
        ResourceID fontID2 = resourceManager->findResource<Font>("PAPYRUS");
        
        Font& uiFont2Ref = resourceManager->getFont(fontID2);
        Font& uiFontRef = resourceManager->getFont(fontID);
        uiFont = &uiFontRef;
        uiFont2 = &uiFont2Ref;
    }

    // Wire text shader into the renderer (renderer controls projection per-viewport)
    renderer->setTextShader(*textShader);

    prefabManager = std::make_unique<PrefabManager>(*ecs);
    prefabManager->Load_Prefab(prefabTestPath.c_str());

    EntityID animal = prefabManager->Instantiate("animal");
    EntityID animal_2 = prefabManager->Instantiate("animal");
    logger.log("Created animal entity with ID: " + std::to_string(animal) + " and " + std::to_string(animal_2));

    imGuiManager = std::make_unique<ImGuiManager>(
        m_window, *renderer,
        *inputSystem, physicsSystem,
        *audioSystem, *resourceManager,
        *gsm);

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





















#if 0
int Application::run() {
	// 1) read JSON (with safe defaults)
	//const WindowConfig c = Config::LoadWindowConfig("assets/config.json");
	const GameConfig c = loadConfig("assets/config.json");

	// 2) create the window
	WindowDesc wd;
	wd.width = c.window.width;
	wd.height = c.window.height;
	wd.title = c.window.title;
	wd.vsync = c.window.vsync;

	if (!m_window.create(wd)) return -1;

	// Prime the window so theres no white flash before the first real frame
	glViewport(0, 0, wd.width, wd.height);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	m_window.swap();

	// set up a simple 2D projection (pixel coords, origin = top-left) for the OS window
	int currW = wd.width, currH = wd.height;
	glfwGetFramebufferSize(m_window.handle(), &currW, &currH);
	glViewport(0, 0, currW, currH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, currW, currH, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// sensible 2D state for the OS window (ImGui draws here later)
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Messaging::MessageBus bus;

	Input      game_input_obj(m_window.handle());
	game_input_obj.SetMessageBus(&bus);
	ECS& ecs = ECS::getECS();
	ResourceManager resourceManager(ecs);
	AudioSystem audioSystem("AudioSystem", ecs, resourceManager);
	resourceManager.setAudioSystem(&audioSystem);

    resourceManager.loadResources("audio");
	resourceManager.loadResources("shaders");
	resourceManager.loadResources("textures");
	resourceManager.loadResources("fonts");
	resourceManager.loadResources("prefabs");
	resourceManager.loadResources("scenes");

	Physics    game_physics_obj;
	Game_Loop  game_loop_obj(m_window.handle());

    std::string vertexPath,
				fragmentPath,
				prefabPath, prefabTestPath,
				textShaderVertexPath, textShaderFragmentPath,
				scenePath;

	auto shaderPath = resourceManager.getFilePathsForKey<ShaderFile>("background");
	for (auto const& path : shaderPath) {
		std::filesystem::path p{ path };
		auto ext = p.extension().string();
		if (ext == ".vert") vertexPath = p.string();
		else if (ext == ".frag") fragmentPath = p.string();	
	}

	auto textShaderPaths = resourceManager.getFilePathsForKey<ShaderFile>("text");
	for (auto const& path : textShaderPaths) {
		std::filesystem::path p{ path };
		auto ext = p.extension().string();
		if (ext == ".vert") textShaderVertexPath = p.string();
		else if (ext == ".frag") textShaderFragmentPath = p.string();
	}

	auto prefabPaths = resourceManager.getFilePathsForKey<PrefabFile>("prefabs");
	if (!prefabPaths.empty()) prefabPath = prefabPaths.front();

	auto testingPrefabPaths = resourceManager.getFilePathsForKey<PrefabFile>("testing_prefabs");
	if (!testingPrefabPaths.empty()) prefabTestPath = testingPrefabPaths.front();

	auto scenePaths = resourceManager.getFilePathsForKey<SceneFile>("scenes");
	if (!scenePaths.empty()) scenePath = scenePaths.front();

    bus.Register(&game_loop_obj);

	TextureStore textures;
	GameManager gsm(ecs);
	gsm.load(prefabPath.c_str(), scenePath.c_str(), textures);
	Renderer   renderer(vertexPath.c_str(), fragmentPath.c_str());
   
	//initialise text rendering system
	Shader textShader(textShaderVertexPath.c_str(), textShaderFragmentPath.c_str());

	ResourceID fontID = resourceManager.findResource<Font>("NanumBrushScript-Regular");
	Font& uiFont = resourceManager.getFont(fontID);

	textShader.user_Shader_program();
	textShader.SET_uniform_variable("text", 0);


	glm::mat4 projection = glm::ortho(
		0.0f, static_cast<float>(wd.width),
		0.0f, static_cast<float>(wd.height)
	);
	textShader.user_Shader_program();
	glUniformMatrix4fv(
		glGetUniformLocation(textShader.get_programID(), "projection"),
		1, GL_FALSE, &projection[0][0]
	);

	// Initialize
	PrefabManager prefabManager(ecs);

	// Load prefabs
	prefabManager.Load_Prefab(prefabTestPath.c_str());

	EntityID animal = prefabManager.Instantiate("animal");
	EntityID animal_2 = prefabManager.Instantiate("animal");
	std::cout << animal << animal_2 << "\n";

	//// Focus roughly on the flamingo (center near its JSON x/y+w/2,h/2)
	//Cam2D_Enable("Flamingo", true);
	//Cam2D_SetCenter("Flamingo", 0.10f + 0.26f * 0.5f, 0.44f + 0.50f * 0.5f); // ~ (0.23, 0.69)
	//Cam2D_SetZoom("Flamingo", 1.8f);


	ImGuiManager imGuiManager(m_window, renderer,
		game_input_obj, game_physics_obj,
		audioSystem, resourceManager,
		gsm);

	while (!m_window.shouldClose()) {
		m_window.poll();

		double dt = game_loop_obj.calculate_DT();
		game_loop_obj.display_FPS(m_window.handle(), c.window.title, dt);
		game_input_obj.update_mouse_pos(m_window.handle());

		// Keyboard scene switching
		// Keyboard scene switching (1..9 + ESC)
		for (int k = GLFW_KEY_1; k <= GLFW_KEY_9; ++k)
			if (game_input_obj.WasPressed(k))
				gsm.handleKeyPress(k);

		if (game_input_obj.WasPressed(GLFW_KEY_ESCAPE))
			gsm.handleKeyPress(GLFW_KEY_ESCAPE);

		// Process per-frame hotkeys (zoom/pan for Flamingo etc.)
		game_input_obj.ProcessHotkeys(gsm);


		// Clear OS window (ImGui draws on top; scene is inside the Viewport FBO)
		int fbw = 0, fbh = 0; glfwGetFramebufferSize(m_window.handle(), &fbw, &fbh);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, fbw, fbh);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Build and render all ImGui windows (Viewport + overlays)
		Gui::updateImGuiManager(dt, imGuiManager);

		//---------render text overlay----------
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, fbw, fbh);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Draw your text last (so it's on top)
		uiFont.RenderText(
			textShader,
			"Nocturnal Studio Engine",
			25.0f, wd.height - 50.0f, //move near top
			1.0f,
			glm::vec3(1.0f, 0.0f, 0.2f) //red text
		);
		uiFont2.RenderText(
			textShader,
			"Luminara Zoo",
			wd.width/2.f,wd.height/2.f,
			0.8f,
			glm::vec3(0.6f, 0.2f, 0.8f) //purple text
		);
		//----------end text overlay----------


		// Application.cpp 
		if (game_input_obj.WasMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
			ImVec2 rmin, rmax;
			imGuiManager.getViewportRectWindow(rmin, rmax);

			// mouse in GLFW client coords (pixels)
			Vector2D mp = game_input_obj.GET_mouse_pos(false, 0, 0);

			if (mp.x() >= rmin.x && mp.x() <= rmax.x &&
				mp.y() >= rmin.y && mp.y() <= rmax.y) {

				float nx = float((mp.x() - rmin.x) / (rmax.x - rmin.x));
				float ny = float((mp.y() - rmin.y) / (rmax.y - rmin.y));

				// because you draw the Image with flipped V (uv (0,1)->(1,0))
				ny = 1.0f - ny;

				gsm.handleClick(nx, ny);
			}
		}



		// Quit button in JSON
		if (gsm.popQuitRequested())
			glfwSetWindowShouldClose(m_window.handle(), 1);

		game_input_obj.EndFrame();
		m_window.swap();
	}

	for (auto& [id, font] : resourceManager.getFonts()) {
		font.Cleanup();
	}
	
	m_window.destroy();
	return 0;
}
#endif