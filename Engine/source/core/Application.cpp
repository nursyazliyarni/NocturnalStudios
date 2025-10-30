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
// init(), run(), shutdown()
#include "core/Application.h"
#include "core/Config.h"
#include <GLFW/glfw3.h>            // glViewport/glClearColor/glClear
#include <cmath>                   // cosf, sinf
#include <glm/gtc/constants.hpp>   // glm::pi<float>()
#include <chrono>
#include "math/Vector.h"
#include "core/ImGUIManager.h"
#include "physics/AABB.h"
#include "ECS/ECS.h"
#include "resources/ResourceManager.h"
#include "core/GameManager.h"
#include "scene/Prefab.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace {
	constexpr float TWO_PI = 2.0f * glm::pi<float>();
	constexpr int   CIRCLE_SEGMENTS = 64;
	using Clock = std::chrono::high_resolution_clock;
}

bool isDragging = false; //is a sprite being dragged
int draggedSpriteIndex = -1; //index into sprite_vector
int duplicateIndex = -1; //where clone is in the vector
Vector2D dragOffset(0.0f, 0.0f); //mouse offset from top-left of sprite

//Application::Application() = default;
Application::Application() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //enable memory leak check at program exit
}

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



	Input      game_input_obj(m_window.handle());
	ECS& ecs = ECS::getECS();
	ResourceManager resourceManager(ecs);
	AudioSystem audioSystem("AudioSystem", ecs, resourceManager);
	resourceManager.setAudioSystem(&audioSystem);

    resourceManager.loadResources("audio");
	resourceManager.loadResources("shaders");
	resourceManager.loadResources("textures");
	resourceManager.loadResources("prefabs");
	resourceManager.loadResources("scenes");

	Physics    game_physics_obj;
	Game_Loop  game_loop_obj(m_window.handle());

    std::string vertexPath,
				fragmentPath,
				prefabPath, prefabTestPath,
				scenePath;

	auto shaderPath = resourceManager.getFilePathsForKey<ShaderFile>("background");
	for (auto const& path : shaderPath) {
		std::filesystem::path p{ path };
		auto ext = p.extension().string();
		if (ext == ".vert") vertexPath = p.string();
		else if (ext == ".frag") fragmentPath = p.string();	
	}

	auto prefabPaths = resourceManager.getFilePathsForKey<PrefabFile>("prefabs");
	if (!prefabPaths.empty()) prefabPath = prefabPaths.front();

	auto testingPrefabPaths = resourceManager.getFilePathsForKey<PrefabFile>("testing_prefabs");
	if (!testingPrefabPaths.empty()) prefabTestPath = testingPrefabPaths.front();

	auto scenePaths = resourceManager.getFilePathsForKey<SceneFile>("scenes");
	if (!scenePaths.empty()) scenePath = scenePaths.front();


	TextureStore textures;
	GameManager gsm(ecs);
	gsm.load(prefabPath.c_str(), scenePath.c_str(), textures);
	Renderer   renderer(vertexPath.c_str(), fragmentPath.c_str());
   
	// Initialize
	PrefabManager prefabManager(ecs);

	// Load prefabs
	prefabManager.Load_Prefab(prefabTestPath.c_str());

	EntityID animal = prefabManager.Instantiate("animal");
	EntityID animal_2 = prefabManager.Instantiate("animal");
	std::cout << animal << animal_2 << "\n";




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


		// Clear OS window (ImGui draws on top; scene is inside the Viewport FBO)
		int fbw = 0, fbh = 0; glfwGetFramebufferSize(m_window.handle(), &fbw, &fbh);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, fbw, fbh);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Build and render all ImGui windows (Viewport + overlays)
		Gui::updateImGuiManager(dt, imGuiManager);


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



	m_window.destroy();
	return 0;
}
