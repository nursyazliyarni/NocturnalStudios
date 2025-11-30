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
		- Full Screen mode toggle

	Secondary Author:
		Chang Qi Xuan Celest (35%)
		- Integrated drag and drop functionality for sprites.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "core/Application.h"

#include <vector>

#include <algorithm>
#include <cmath>

#include "scene/Prefab.h"

#include <cmath>
#include <algorithm>
#include "ECS/ECS.h"
#include "ECS/ECS_Component.h"
#include "ECS/ECS_DEF.h"
#include "resources/ResourceManager.h"
#include "scene/Prefab.h" // Prefab_SetTextureLookup, Scenes_GetInitialName, Prefab_SpawnEntitiesForScene

#include <unordered_map>
#include <cstdlib>
#include <ctime>

#define UNUSED(x) (void)(x)  // To suppress unused variable warnings

extern void Prefab_SetTextureLookup(std::function<int(const std::string&)>);

// --- Very simple random drift for any entity with AI + Physics (no fancy RNG) ---
static void UpdateRandomDrift(ECS& ecs, double dt) {
	if (dt <= 0.0) return;

	// Seed std::rand once
	static bool s_seeded = false;
	if (!s_seeded) { std::srand(static_cast<unsigned>(std::time(nullptr))); s_seeded = true; }

	// Per-entity drift state (heading + timer for next small turn)
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

		// Init per-entity state if needed
		DriftState& st = s_state[e];
		if (st.nextTurn <= 0.0f && st.heading == 0.0f) {
			st.heading = frand(0.0f, 6.2831853f);   // random angle in [0, 2?)
			st.nextTurn = frand(0.6f, 1.8f);        // seconds until next turn
		}

		// Count down & occasionally change heading slightly
		st.nextTurn -= static_cast<float>(dt);
		if (st.nextTurn <= 0.0f) {
			st.heading += frand(-0.9f, 0.9f);       // small random turn
			if (st.heading > 6.2831853f) st.heading -= 6.2831853f;
			if (st.heading < 0.0f)       st.heading += 6.2831853f;
			st.nextTurn = frand(0.6f, 1.8f);
		}

		// Desired velocity points along the heading at maxSpeed
		float desiredVx = std::cos(st.heading) * ai->maxSpeed;
		float desiredVy = std::sin(st.heading) * ai->maxSpeed;

		// Accel toward desired vel, limited by ai->accel * dt
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

		// Clamp speed to maxSpeed
		float v2 = pc->velocity.x() * pc->velocity.x() + pc->velocity.y() * pc->velocity.y();
		float ms = ai->maxSpeed;
		if (v2 > ms * ms && v2 > 0.0f) {
			float k = ms / std::sqrt(v2);
			pc->velocity.x() *= k; pc->velocity.y() *= k;
		}

		// Integrate position in normalized [0..1] space, bounce at edges
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

namespace {
	constexpr float TWO_PI = 2.0f * glm::pi<float>();
	constexpr int   CIRCLE_SEGMENTS = 64;
	using Clock = std::chrono::high_resolution_clock;

	bool isDragging = false; //is a sprite being dragged
	int draggedSpriteIndex = -1; //index into sprite_vector
	int duplicateIndex = -1; //where clone is in the vector
	Vector2D dragOffset(0.0f, 0.0f); //mouse offset from top-left of sprite

	static Application* s_App = nullptr;
	static void App_OnGlfwFocus(GLFWwindow* window, int focused) {
		UNUSED(window);
		if (s_App) s_App->onFocusChanged(focused == GLFW_TRUE);
	}

	static void App_OnGlfwIconify(GLFWwindow* window, int iconified) {
		UNUSED(window);
		if (s_App) s_App->onIconifyChanged(iconified == GLFW_TRUE);
	}
	static void App_OnGlfwDrop(GLFWwindow* window, int count, const char** paths) {
		UNUSED(window);
		if (s_App) s_App->onFilesDropped(count, paths);
	}
}

//Application::Application() = default;
Application::Application() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //enable memory leak check at program exit
}

void Application::onFocusChanged(bool focused) {
	if (!focused) {
		if (autoMinimizeOnFocusLoss) glfwIconifyWindow(m_window.handle());

		if (audioSystem) {
			audioSystem->pauseAllSFX();
			audioSystem->pauseAllBGM();
		}

		if (gsm) gsm->pauseGame();
		if (inputSystem) inputSystem->clearAllStates();
	}
	else {
		if (glfwGetWindowAttrib(m_window.handle(), GLFW_ICONIFIED)) 
			glfwRestoreWindow(m_window.handle());

		if (autoResumeOnFocusGain) {
			if (audioSystem) {
				audioSystem->resumeAllSFX();
				audioSystem->resumeAllBGM();
			}
			if (gsm && gsm->isPaused()) gsm->resumeGame();
		}

		if (inputSystem) inputSystem->EndFrame();
	}
}

void Application::onIconifyChanged(bool iconified) {
	if (iconified) {
		if (audioSystem) {
			audioSystem->pauseAllSFX();
			audioSystem->pauseAllBGM();
		}

		if (gsm) gsm->pauseGame();
		if (inputSystem) inputSystem->clearAllStates();
	}
	else {
		if (autoResumeOnFocusGain) {
			if (audioSystem) {
				audioSystem->resumeAllSFX();
				audioSystem->resumeAllBGM();
			}
			if (gsm && gsm->isPaused()) gsm->resumeGame();
		}

		if (inputSystem) inputSystem->EndFrame();
	}
}


void Application::init() {
	logger.setup_crash_handler();
	const GameConfig c = loadConfig("assets/config.json");
	windowTitle = c.window.title;
	imGUIVisible = c.imgui_enabled;

    WindowDesc wd;
    wd.width = c.window.width;
    wd.height = c.window.height;
    wd.title = c.window.title;
    wd.vsync = c.window.vsync;
	wd.fullscreen = true;

    if (!m_window.create(wd)) {
        logger.error("Failed to create window");
        isInitialized = false;
        return;
    }

	s_App = this;
	glfwSetWindowFocusCallback(m_window.handle(), App_OnGlfwFocus);
	glfwSetWindowIconifyCallback(m_window.handle(), App_OnGlfwIconify);
	glfwSetDropCallback(m_window.handle(), App_OnGlfwDrop);

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


	// Wire prefab sprite_path -> GL texture id using your TextureStore API.
// Must be set BEFORE any Prefab/Scene instantiation.
	Prefab_SetTextureLookup([this](const std::string& p) -> int {
		// robust to presence/absence of "assets/"
		std::string path = p;
		if (path.rfind("assets/", 0) != 0) path = "assets/" + path;
		return static_cast<int>(textures.get(path));
		});

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
	prefabManager->Load_Prefab("assets/prefabs/prefabs.json");

    gameLoop = std::make_unique<Game_Loop>(m_window.handle());
    bus.Register(gameLoop.get());
;
	
    textShader = std::make_unique<Shader>(textShaderVertexPath.c_str(), textShaderFragmentPath.c_str());

    // Wire text shader into the renderer (renderer controls projection per-viewport)
    renderer->setTextShader(*textShader);
	gsm = std::make_unique<GameManager>(*ecs,textures, c.imgui_enabled);
	gsm->loadAll(*prefabManager, scenePath.c_str(), textures);
	// Spawn the initial scene (from scenes.json -> states.initial)
	{
		const std::string initialScene = Scenes_GetInitialName();
		if (!initialScene.empty()) {
			gsm->populateScene(initialScene, *prefabManager);
			logger.log("[SceneSpawn] Initial scene spawned: " + initialScene);
		}
	}

	// When GameManager activates a new scene (keyboard or button 'goto'), respawn from JSON
	gsm->setSceneChangedCallback([this](const std::string& sceneName) {
        // Stop non-persistent audio before wiping entities.
        if (audioSystem) {
            // false => keep sounds with persist_on_scene_change == true
            audioSystem->deleteAllAudioInstances(false);
        }

		gsm->reloadAllScene(textures);
        // Clear all current entities.
        //auto all = ecs->getAllEntities();
        //for (auto id : all) ecs->deleteEntity(id);

        // Respawn from scenes.json.
		gsm->populateScene(sceneName, *prefabManager);
        logger.log(std::string{ "[SceneSpawn] Activated -> " } + sceneName);
    });

	if (!imGUIVisible)renderer->initFBO(wd.width, wd.height);

	imGuiManager = std::make_unique<ImGuiManager>(
		m_window, *renderer,
		*inputSystem, physicsSystem,
		*audioSystem, *resourceManager,
		*gsm, *logic);

	Logic_Factory::get_instance().add_logic("FlamingoLogic", []() {
		return std::make_unique<FlamingoLogic>();
	});

	/*LogicRegistry::instance().register_logic("HorseLogic", []() {
		return std::make_unique<HorseLogic>();
		});
	LogicRegistry::instance().register_logic("SealLogic", []() {
		return std::make_unique<SealLogic>();
		});
	LogicRegistry::instance().register_logic("OwlLogic", []() {
		return std::make_unique<OwlLogic>();*/

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
	if (imGUIVisible)imGuiManager->recordFirstUndoState();
    while (!m_window.shouldClose()) {
        m_window.poll();

		double dt = gameLoop->calculate_DT();

		if (gsm && !gsm->isPaused()) {
			gsm->update(static_cast<float>(dt), *renderer, textures);
		}
		if (imGUIVisible)Gui::updateImGuiManager(dt, *imGuiManager);
		gameLoop->display_FPS(m_window.handle(), windowTitle, dt);
		inputSystem->update_mouse_pos(m_window.handle());

		if (ecs) UpdateRandomDrift(*ecs, dt);

		// Debug: log firefly state twice per second to confirm it's updating
		static double dbgAccum = 0.0;
		dbgAccum += dt;

		if (dbgAccum > 0.5 && ecs) {
			dbgAccum = 0.0;
			if (EntityID ff = ecs->findEntityByName("firefly_1"); ff != INVALID_ENTITY_ID) {
				if (auto* t = ecs->getComponent<Transform>(ff)) {
					if (auto* pc = ecs->getComponent<PhysicsComponent>(ff)) {
						logger.log("FF pos=(" + std::to_string(t->position.x()) + "," + std::to_string(t->position.y()) + ")" +
							" vel=(" + std::to_string(pc->velocity.x()) + "," + std::to_string(pc->velocity.y()) + ")");
					}
				}
			}
			// else: stay quiet when not in Owl scene
		}

		if (!gsm || !gsm->isPaused()) {
			for (int k = GLFW_KEY_1; k <= GLFW_KEY_9; ++k)
            	if (inputSystem->WasPressed(k))
                	gsm->switchToSceneForKey(k); // respects AppState::Playing

			if (inputSystem->WasPressed(GLFW_KEY_F))
				gsm->handleKeyPress(GLFW_KEY_F);

        	if (inputSystem->WasPressed(GLFW_KEY_ESCAPE))
            	gsm->handleKeyPress(GLFW_KEY_ESCAPE);

			if (inputSystem->WasPressed(GLFW_KEY_F11)) 
				m_window.toggleFullscreen();

        	inputSystem->ProcessHotkeys(*gsm);
        	TransformSystem::TransformInput(*inputSystem, dt);
		}

        int fbw = 0, fbh = 0; glfwGetFramebufferSize(m_window.handle(), &fbw, &fbh);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, fbw, fbh);
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.06f, 0.06f, 0.07f, 1.0f);

        //Prefab_SetTextureLookup([this](const std::string& p) -> int {
            //return static_cast<int>(textures.get(p));
        //});

        glClear(GL_COLOR_BUFFER_BIT);

		if (imGUIVisible)Gui::updateImGuiManager(dt, *imGuiManager);
		else {

			int winheight,winwidth;
			m_window.getWindowSize(winwidth, winheight);
			gsm->imGUIlessRender(*renderer, *inputSystem, m_window.handle(), winwidth,winheight,dt,*resourceManager);

			double mouse_x, mouse_y;
			glfwGetCursorPos(m_window.handle(), &mouse_x, &mouse_y);
			
			float nmouse_x{ static_cast<float>(mouse_x) / static_cast<float>(winwidth) }, nmouse_y{ static_cast<float>(mouse_y) / static_cast<float>(winheight) };
		
				logic->check_animal_status(*gsm);
				
				auto drag_value = logic->check_item_status(*gsm,{nmouse_x,nmouse_y},*inputSystem, m_window.handle());
				logic->update_animal_ALLstate(*gsm, dt);
				logic->drag_drop(*gsm, { nmouse_x,nmouse_y }, *inputSystem, m_window.handle(),drag_value);
				
		}

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, fbw, fbh);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Text overlay is now rendered by Renderer inside the viewport FBO (see ImGuiManager)

        if (inputSystem->WasMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
			ImVec2 rmin{}, rmax;

			if (imGUIVisible)imGuiManager->getViewportRectWindow(rmin, rmax);
			else { //uses window size to dtermine click coords
				int maxx, maxy;
				m_window.getWindowSize(maxx, maxy);
				rmax={ static_cast<float>(maxx) ,static_cast<float>(maxy) };
			}
			
			Vector2D mp = inputSystem->GET_mouse_pos(false, 0, 0);
			if (mp.x() >= rmin.x && mp.x() <= rmax.x &&
				mp.y() >= rmin.y && mp.y() <= rmax.y) {
				float nx = float((mp.x() - rmin.x) / (rmax.x - rmin.x));
				float ny = float((mp.y() - rmin.y) / (rmax.y - rmin.y));

				//ny = 1.0f - ny;
				gsm->handleClick(nx, ny);

				if (gsm->popQuitRequested()) {
					glfwSetWindowShouldClose(m_window.handle(), GLFW_TRUE);
					break;
				}
			}
            
        }
		TransformSystem::TransformUpdate();

		if (inputSystem->WasPressed(GLFW_KEY_P))
		{
			
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

void Application::onFilesDropped(int count, const char** paths) {
    if (!resourceManager) return;
    int loadedAudio = 0;
    int loadedTex = 0;
    for (int i = 0; i < count; ++i) {
        std::filesystem::path p{ paths[i] };
        if (!p.has_extension()) continue;
        std::string ext = p.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        std::string stem = p.stem().string();
        std::string full = p.string();
        // Accept textures
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga" || ext == ".gif" || ext == ".hdr" || ext == ".psd") {
            resourceManager->loadTexture(stem, full);
            ++loadedTex;
        }
		// Accept audio
		else if (ext == ".wav" || ext == ".ogg" || ext == ".mp3" || ext == ".flac" || ext == ".aiff" || ext == ".fsb")
		{
			std::filesystem::path src = p;
			std::string stem = src.stem().string();
			std::string full = src.string();

			//estimate file size to determine if BGM or SFX
			std::uintmax_t bytes = 0;
			try {
				bytes = std::filesystem::file_size(src);
			}
			catch (...) {
				//if can't get size, default to SFX
				bytes = 0;
			}

			bool isBGM = (bytes > 1 * 1024 * 1024); //1MB threshold
			//auto-detects BGM or SFX.
			std::string key;
			if (isBGM)
				key = "BGM_" + stem;
			else
				key = "SFX_" + stem;
			std::string pathToUse = full;
			try {
				std::filesystem::path destFolder = "assets/audio";
				std::filesystem::create_directories(destFolder);

				std::filesystem::path destPath = destFolder / src.filename();
				std::filesystem::copy_file(
					src, destPath,
					std::filesystem::copy_options::overwrite_existing);

				pathToUse = destPath.string();
				logger.log("[DragDrop] Copied audio to " + pathToUse);
			}
			catch (const std::exception& e) {
				logger.warning(std::string("[DragDrop] copy_file failed, using original audio path. Reason: ")
					+ e.what());
			}
			try {
				resourceManager->loadAudio(key, pathToUse);
				++loadedAudio;
				logger.log("[DragDrop] Loaded " +
					std::string(isBGM ? "BGM" : "SFX") +
					" audio '" + key + "' from " + pathToUse +
					" (" + std::to_string(bytes / 1024) + " KB)");
			}
			catch (const std::exception& e) {
				logger.error(std::string("[DragDrop] Failed to load audio resource: ") + e.what());
			}
		}
    }
    if (loadedTex || loadedAudio) {
        logger.log("[DragDrop] Added " + std::to_string(loadedTex) + " textures, " + std::to_string(loadedAudio) + " audio assets.");
    } else {
        logger.warning("[DragDrop] No supported assets dropped.");
    }
}
