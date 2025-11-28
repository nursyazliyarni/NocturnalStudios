/*!------------------------------------------------------------------------------------------------
	File: ImGUIManager.cpp
	Project: NocturnalStudio

	This file contains the implementation of the ImGuiManager class, which manages the integration
	of the ImGui library into the customize game engine. It handles the initialization,
	frame updates, and rendering of ImGui components, as well as providing various UI elements
	for debugging and interaction within the engine.

	Primary Author:
		Lau Xiang Yang (40%)
		- Created the ImGuiManager class and its specializations.

	Secondary Author:
		Rishika Bimal Attodi (30%)
		- Integrated drawAllSprites and drawViewportShapes

	Secondary Author:
		Nur Syazliyarni (30%)
		- Added the debug drawer that will load under showViewportWindowUI
		- Editor able to list and edit transform game objects in current scene
		- Added assets browser and layout of bottom editor dock

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "../include/core/ImGuiManager.h"
#include <chrono>
#include "graphics/DebugDraw.h"
#include "core/GameManager.h"

#include "graphics/Camera2D.h" 
#include "core/TimeUtils.h"
#include <filesystem>
#include <algorithm>

namespace {
	using Clock = std::chrono::high_resolution_clock;
}

void Gui::updateImGuiManager(double& deltaTime, ImGuiManager& imGuiManager) {
	// update delta/window info
	imGuiManager.updateDeltaTime(deltaTime);

	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	imGuiManager.updateWindowTopLeft(mainViewport->Pos);
	imGuiManager.updateWindowBottomRight({ mainViewport->Pos.x + mainViewport->Size.x, mainViewport->Pos.y + mainViewport->Size.y });

	// start ImGui frame (needed before creating windows)
	imGuiManager.startFrame();

	//create dockspace
	imGuiManager.createDockspace();

	// Render the viewport first — this contains renderer/physics/logic and measures those times itself.
	imGuiManager.showViewportWindowUI();

	// Measure only the remaining ImGui overhead (exclusive UI time)
	double imGuiSec = TimeUtils::MeasureSeconds([&]() {
		imGuiManager.showMousePosOverlayUI();
		imGuiManager.showPerformanceOverlayUI();
		imGuiManager.showTransformUI();
		imGuiManager.draw_logic_selector();
		imGuiManager.showEntityListUI();
		//imGuiManager.showLogsOverlayUI();
		imGuiManager.showBottomDockUI();
		});

	// finish frame and record the exclusive ImGui time
	imGuiManager.endFrame();
	imGuiManager.setSystemTimings(imGuiSec);
}

ImGuiManager::ImGuiManager(Window& w, Renderer& r,
	Input& i, Physics& p,
	AudioSystem& a, ResourceManager& rm,
	GameManager& g, Animal_Logic& l)
	: mainWindow{ w }, renderer{ r },
	input{ i }, physics{ p }, audioSystem{ a }, resourceManager{ rm },
	gsm{ g }, logic{ l }, windowTopLeft{}, windowBottomRight{},
	deltaTime{ 0.0 }, imGuiTime{ 0.0 }, inputTime{ 0.0 },
	logicTime{ 0.0 }, renderTime{ 0.0 }, physicsTime{ 0.0 },
	selectedEntityType{ 0 }, activeInstances{},
	m_editorMode{ EditorMode::EDIT }  // Initialize to EDIT mode
{
	ImGui::CreateContext();

	// Enable docking
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = false;  // Dock without holding Shift

#ifdef IMGUI_SAVE_LAYOUT
	io.IniFilename = "editor_layout.ini";  // Save layout
#else
	io.IniFilename = nullptr;  // Don't save layout
#endif

	ImGui_ImplGlfw_InitForOpenGL(mainWindow.handle(), /*install_callbacks=*/false);
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGui::StyleColorsClassic();
	logger.log("ImGui initialized successfully.");
}

ImGuiManager::~ImGuiManager() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiManager::updateWindowTopLeft(const ImVec2& topLeft) {
	windowTopLeft = topLeft;
}

void ImGuiManager::updateWindowBottomRight(const ImVec2& bottomRight) {
	windowBottomRight = bottomRight;
}

void ImGuiManager::updateDeltaTime(double& dt) {
	deltaTime = dt;
}

double ImGuiManager::getDeltaTime(double& dt) const {
	dt = deltaTime; return dt;
}

ImVec2 ImGuiManager::get_mousePOS(bool is_normalized) {
	// Get current window size
	ImVec2 windowSize = ImGui::GetWindowSize();
	ImVec2 windowPos = ImGui::GetWindowPos();
	// Get mouse position
	ImVec2 mousePos = ImGui::GetMousePos();

	if (is_normalized) {
		// Normalize to [0, 1] relative to this window
		float normalizedX = mousePos.x / windowSize.x;
		float normalizedY = mousePos.y / windowSize.y;

		// Clamp to [0, 1] range
		normalizedX = std::clamp(normalizedX, 0.0f, 1.0f);
		normalizedY = std::clamp(normalizedY, 0.0f, 1.0f);
		return { normalizedX,normalizedY };
	}

	else {
		return { mousePos.x,mousePos.y };
	}
}

void ImGuiManager::startFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::endFrame() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

Vector2D ImGuiManager::getWindowTopLeft() const {
	return Vector2D(windowTopLeft.x, windowTopLeft.y);
}

Vector2D ImGuiManager::getWindowBottomRight() const {
	return Vector2D(windowBottomRight.x, windowBottomRight.y);
}

void ImGuiManager::setSystemTimings(double imGuITime) {
	imGuiTime = imGuITime;
}

void ImGuiManager::setRenderTime(double time) {
	renderTime = time;
}

void ImGuiManager::setPhysicTime(double time) {
	physicsTime = time;
}

void ImGuiManager::setInputTime(double time) {
	inputTime = time;
}

//void ImGuiManager::showViewportWindowUI() {
//	constexpr float overlayWidthOffset = 300.0f;
//	constexpr float overlayHeightOffset = 200.0f;
//
//	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize;
//
//	// Calculate the size of the viewport window
//	float viewportWidth = windowBottomRight.x - windowTopLeft.x - overlayWidthOffset;
//	float viewportHeight = windowBottomRight.y - windowTopLeft.y - overlayHeightOffset;
//
//	// Position the viewport window at the top-left corner of the main window
//	ImGui::SetNextWindowPos(windowTopLeft);
//	ImGui::SetNextWindowSize(ImVec2(viewportWidth, viewportHeight));
//	ImGui::Begin("Viewport", nullptr, windowFlags);
//
//	// ?? ADD: actual drawable region (excludes title/padding)
//	const ImVec2 avail = ImGui::GetContentRegionAvail();
//	const int vw = (int)std::max(1.0f, avail.x);
//	const int vh = (int)std::max(1.0f, avail.y);
//
//	 static int lastW = -1, lastH = -1;
//    if (vw != lastW || vh != lastH) {        // recreate only when needed
//        //renderer.initFBO(vw, vh);
//        lastW = vw; lastH = vh;
//    }
//
//	auto physicsStart = Clock::now();
//
//	physics.update_sprite_movement(renderer, input);
//	physics.update_sprite_scaling(renderer, input);
//	physics.update_sprite_rotation(renderer, input);
//
//	auto physicsEnd = Clock::now();
//	double physicsTime = std::chrono::duration<double>(physicsEnd - physicsStart).count();
//	setPhysicTime(physicsTime);
//
//	auto render_start = Clock::now();
//
//	// Initialize and bind FBO for rendering
//	// renderer.initFBO(viewportHeight, viewportWidth);            // REMOVE this (args swapped)
//	renderer.initFBO(vw, vh);                                      // ADD: correct (width, height)
//	renderer.bindFBO();
//
//	// ?? ADD: set viewport + orthographic to pixel-space for the FBO
//	renderer.resize(vw, vh);
//
//	// You can keep your clear (fine for 2D)
//	renderer.clear(0.1f, 0.1f, 0.12f);
//
//	// @Todo (axewhy): Render the actual viewport content here, e.g., using a framebuffer texture
//	// Render background to FBO
//	renderer.background_render(texture.get_textureID(0), "bg_texture");
//
//	auto input_start = Clock::now();
//
//	input.updateRotations();
//
//	// behind-sprites pass (scaled)
//	//renderer.drawAllSprites(texture, { 3,4 }, /*behindPass=*/true,  input.getSpriteScalesPtr());
//	renderer.drawAllSprites(texture, { 3,4 ,5}, true,
//		input.getSpriteScalesPtr(),
//		input.getSpriteRotationsPtr(),
//		vw, vh);
//
//	//draw all SHAPES from Renderer.cpp (keeps ImGui clean of GL) - rishika
//	renderer.drawViewportShapes(vw, vh);
//
//	// front-sprites pass (scaled)
//	//renderer.drawAllSprites(texture, { 3,4 }, /*behindPass=*/false, input.getSpriteScalesPtr());
//	renderer.drawAllSprites(texture, { 3,4,5 }, false,
//		input.getSpriteScalesPtr(),
//		input.getSpriteRotationsPtr(),
//		vw, vh);
//
//	renderer.unbindFBO();
//	auto input_end = Clock::now();
//	double inputTime = std::chrono::duration<double>(input_end - input_start).count();
//	setInputTime(inputTime);
//
//	auto render_end = Clock::now();
//	double renderTime = std::chrono::duration<double>(render_end - render_start).count();
//	setRenderTime(renderTime);
//
//	// show FBO in the panel (use the content size so it matches exactly)
//	// ImGui::Image((void*)(intptr_t)renderer.getFBOTexture(), ImVec2(viewportWidth, viewportHeight));
//	// NEW: flip V when sampling the FBO in ImGui
//	ImGui::Image((ImTextureID)(intptr_t)renderer.getFBOTexture(),
//		ImVec2(viewportWidth, viewportHeight),
//		ImVec2(0, 1), ImVec2(1, 0));
//
//	ImGui::End();
//}


void ImGuiManager::showViewportWindowUI() {
	//constexpr float overlayW = 300.f, overlayH = 200.f;
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize;

	/*float vpW = windowBottomRight.x - windowTopLeft.x - overlayW;
	float vpH = windowBottomRight.y - windowTopLeft.y - overlayH;

	ImGui::SetNextWindowPos(windowTopLeft);
	ImGui::SetNextWindowSize(ImVec2(vpW, vpH));
	ImGui::Begin("Viewport", nullptr, flags);*/
	ImGui::Begin("Viewport", nullptr, flags);

	//global editor undo (ctrl+z)
	ImGuiIO& io = ImGui::GetIO();
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
		undoLastAction();
	}

	const ImVec2 avail = ImGui::GetContentRegionAvail();
	const int vw = (int)std::max(1.0f, avail.x);
	const int vh = (int)std::max(1.0f, avail.y);


	static bool prevF = false;
	int fState = glfwGetKey(mainWindow.handle(), GLFW_KEY_F);
	bool currF = (fState == GLFW_PRESS);

	// on key DOWN edge: toggle
	if (currF && !prevF) {
		m_showFPS = !m_showFPS;
	}
	prevF = currF;

	bool isPlayMode = (m_editorMode == EditorMode::PLAY);

	// Logic timing (animal state update)
	//if (isPlayMode) {
	//	logicTime = TimeUtils::MeasureSeconds([&]() {
	//		logic.check_animal_status(gsm);
	//		auto drag_value = logic.check_item_status(
	//			gsm, get_mousePOS(true), input, mainWindow.handle());
	//		logic.update_animal_ALLstate(gsm, getDeltaTime(deltaTime));
	//		logic.drag_drop(gsm, get_mousePOS(true), input, mainWindow.handle(), drag_value);
	//		});
	//}
	//else {
	//	logicTime = 0.0;
	//}

	// Physics timing scope
	if (isPlayMode) {
		TimeUtils::ScopedTimer t{ physicsTime };
		physics.update_sprite_movement(renderer, input);
		physics.update_sprite_scaling(renderer, input);
		physics.update_sprite_rotation(renderer, input);
	}
	else {
		physicsTime = 0.0;
	}

	// Render timing scope (includes scene render + debug draw + unbind)
	{
		TimeUtils::ScopedTimer t{ renderTime };

		renderer.initFBO(vw, vh);
		renderer.bindFBO();
		renderer.resize(vw, vh);
		renderer.clear(0.06f, 0.06f, 0.07f);

		// Let GameManager draw scene + HUD text
		// In edit mode, pass deltaTime as 0 to prevent game state updates
		gsm.renderWithHUD(
			renderer,
			input,
			mainWindow.handle(),
			vw,
			vh,
			isPlayMode ? deltaTime : 0.0,  // Pass 0 deltaTime in edit mode
			resourceManager,
			m_showFPS
		);

		renderer.unbindFBO();
	}


	//// Show the FBO as an ImGui Image (flipped V)
	//ImVec2 imgMin = ImGui::GetCursorScreenPos();
	//ImGui::Image((ImTextureID)(intptr_t)renderer.getFBOTexture(),
	//ImVec2(static_cast<float>(vw), static_cast<float>(vh)), ImVec2(0, 1), ImVec2(1, 0));
	//ImVec2 imgMax = ImVec2(imgMin.x + vw, imgMin.y + vh);

	//// Handle click -> normalized coords -> GSM
	//// (Input handles clicks; store the rect in window client coords)
	//vpMinWin = ImVec2(imgMin.x - windowTopLeft.x, imgMin.y - windowTopLeft.y);
	//vpMaxWin = ImVec2(imgMax.x - windowTopLeft.x, imgMax.y - windowTopLeft.y);

	//ImGui::End();

	//Show the FBO as an ImGui Image (flipped V)
	ImVec2 imgMin = ImGui::GetCursorScreenPos();
	ImGui::Image(
		(ImTextureID)(intptr_t)renderer.getFBOTexture(),
		ImVec2(static_cast<float>(vw), static_cast<float>(vh)),
		ImVec2(0, 1), ImVec2(1, 0)
	);
	ImVec2 imgMax = ImVec2(imgMin.x + vw, imgMin.y + vh);

	// mouse in viewport-normalised coords [0,1]
	ImVec2 mouseScreen = ImGui::GetMousePos();
	ImVec2 mouseViewport{
		(mouseScreen.x - imgMin.x) / static_cast<float>(vw),
		(mouseScreen.y - imgMin.y) / static_cast<float>(vh)
	};
	mouseViewport.x = std::clamp(mouseViewport.x, 0.0f, 1.0f);
	mouseViewport.y = std::clamp(mouseViewport.y, 0.0f, 1.0f);

	isPlayMode = (m_editorMode == EditorMode::PLAY);

	// Logic timing (animal state update) in PLAY mode only
	if (isPlayMode) {
		logicTime = TimeUtils::MeasureSeconds([&]() {
			logic.check_animal_status(gsm);
			auto drag_value = logic.check_item_status(
				gsm, mouseViewport, input, mainWindow.handle());
			logic.update_animal_ALLstate(gsm, deltaTime);
			logic.drag_drop(
				gsm, mouseViewport, input, mainWindow.handle(), drag_value);
			});
	}
	else {
		logicTime = 0.0;
	}

	//Store rect relative to the main window (you were already doing this)
	vpMinWin = ImVec2(imgMin.x - windowTopLeft.x, imgMin.y - windowTopLeft.y);
	vpMaxWin = ImVec2(imgMax.x - windowTopLeft.x, imgMax.y - windowTopLeft.y);

	//handle click and drag picking on scene items (editor)
	handleViewportPicking(imgMin, imgMax, vw, vh);

	ImGui::End();

}


void ImGuiManager::showMousePosOverlayUI() {
	constexpr float width = 300.0f;
	constexpr float height = 100.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Get mouse position in different coordinate spaces
	// @Todo (axewhy): Implement these when input system is ready
	Vector2D worldPosition;
	Vector2D viewPortPosition;
	Vector2D screenPosition;
	//true->get normalized value, false->unnormalized value
	Vector2D mouse_pos = input.GET_mouse_pos(true, mainWindow.desc().width, mainWindow.desc().height);

	// Position the overlay at the top-right corner of the window
	/*ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y };
	ImGui::SetNextWindowPos(windowTopRightPos);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });*/

	//ImGui::Begin("Overlay", nullptr, windowFlags);
	ImGui::Begin("Overlay", nullptr, windowFlags);
	ImGui::Text("Mouse position: %.2f, %.2f", mouse_pos[0], mouse_pos[1]);
	ImGui::Text("World pos: %.2f, %.2f", worldPosition.x(), worldPosition.y());
	ImGui::Text("View pos: %.2f, %.2f", viewPortPosition.x(), viewPortPosition.y());
	ImGui::Text("Screen pos: %.2f, %.2f", screenPosition.x(), screenPosition.y());
	ImGui::End();
}

void ImGuiManager::showPerformanceOverlayUI() {
	constexpr double secondsToMs = 1000.0;
	constexpr float width = 300.0f;
	constexpr float height = 150.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Position the overlay at the top-right corner of the window, below mouse position overlay
	/*ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y + 100.0f };
	ImGui::SetNextWindowPos(windowTopRightPos);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });*/

	ImGui::Begin("Performance", nullptr, windowFlags);

	// Clamp dt to avoid div-by-zero and negative values
	double dt = deltaTime;
	if (dt <= 0.0) dt = 1e-6; // tiny epsilon when paused or first frame
	ImGui::Text("Current FPS: %.2f (%.2f ms)", 1.0 / dt, dt * secondsToMs);

	// Compute percentages against the frame time (dt)
	auto pct = [dt](double t) -> double { return (t <= 0.0) ? 0.0 : (t / dt) * 100.0; };

	// Known subsystem times
	double imguiMs = imGuiTime * secondsToMs;
	double renderMs = renderTime * secondsToMs;
	double physicsMs = physicsTime * secondsToMs;
	double inputMs = inputTime * secondsToMs;
	double logicMs = logicTime * secondsToMs;

	// Compute an "Other" bucket for time not attributed to known systems
	double knownSec = imGuiTime + renderTime + physicsTime + inputTime + logicTime;
	double otherSec = std::max(0.0, dt - knownSec);
	double otherMs = otherSec * secondsToMs;

	ImGui::Separator();
	ImGui::Text("ImGui: %.2f ms (%.2f%%)", imguiMs, pct(imGuiTime));
	ImGui::Text("Renderer: %.2f ms (%.2f%%)", renderMs, pct(renderTime));
	ImGui::Text("Physics: %.2f ms (%.2f%%)", physicsMs, pct(physicsTime));
	ImGui::Text("Input: %.2f ms (%.2f%%)", inputMs, pct(inputTime));
	ImGui::Text("Logic: %.2f ms (%.2f%%)", logicMs, pct(logicTime));
	ImGui::Text("Other: %.2f ms (%.2f%%)", otherMs, pct(otherSec));

	ImGui::End();
}

void ImGuiManager::showEntityListUI(/* Maybe an enum of paramEntityType, Maybe a Reference to a std::vector of objectList?*/) {
	// Fixed size for entity list window
	constexpr float width = 300.0f;
	constexpr float height = 300.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysVerticalScrollbar;;

	// Position the overlay at the top-right corner of the window, below performance overlay
	/*ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y + 250.0f };
	ImGui::SetNextWindowPos(windowTopRightPos);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });*/

	ImGui::Begin("Entity Picker", nullptr, windowFlags);

	bool isPlayMode = (m_editorMode == EditorMode::PLAY);
	if (isPlayMode) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
		ImGui::Text("Play Mode - Entity creation/deletion disabled");
		ImGui::PopStyleColor();
		ImGui::Separator();
	}

	ImGui::Text("Entity List");

	// if (selectedEntityID == paramEntityIDList::MAX) selectedEntityID = paramEntityIDList::MAX - 1; // we assume paramEntityIDList is an enum with it's last entry as MAX
	// e.g enum paramEntityIDList { ENTITY_A, ENTITY_B, ENTITY_C, MAX };

	// Picker controls
	ImGui::Text("Selected Entity Type: %d", selectedEntityType);
	ImGui::SameLine();
	if (ImGui::Button("-") && selectedEntityType > 0) selectedEntityType--;
	ImGui::SameLine();
	if (ImGui::Button("+")) selectedEntityType++;

	/*
	//disable clone/delete in play mode
	if (isPlayMode) {
		ImGui::BeginDisabled();
	}

	// Clone the selected entity
	if (ImGui::Button("Clone")) {
		// @Todo (axewhy):
		// Call factory function and pass the selected entity ID
		// e.g., EntityFactory::cloneEntity(selectedEntityID);
		// @Todo (sarah) provide cloning function
		logger.log("Cloning entity of Type: " + std::to_string(selectedEntityType));
	}
	ImGui::SameLine();
	// Delete the selected entity
	if (ImGui::Button("Delete")) {
		// @Todo (axewhy):
		// Check if the vector or array containing the entities is not empty or if there are entities to delete
		// e.g., if (!EntityManager::getEntities().empty()) {
		//		// Call destroy function and pass the selected entity ID
		//		// e.g., EntityManager::destroyEntity(selectedEntityID);
		//		}
		logger.log("Deleting entity of Type: " + std::to_string(selectedEntityType));
	}

	if (isPlayMode) {
		ImGui::EndDisabled();
	}
	*/
	// --- build vectors for combo display (cheap, rebuild each frame) ---
	std::vector<ResourceID> audioIDs;
	std::vector<std::string> audioNames;
	audioIDs.reserve(resourceManager.getAudios().size());
	audioNames.reserve(resourceManager.getAudios().size());
	for (auto const& kv : resourceManager.getAudios()) {
		audioIDs.push_back(kv.first);
		audioNames.push_back(kv.second.getKey());
	}

	// BGM selector
	static int bgmIndex = 0;
	static int sfxIndex = 0;

	if (isPlayMode) {
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("Reload audio assets")) {
		auto added = resourceManager.reloadAudiosFromDirectory("assets/audio");
		if (added > 0) {
			audioIDs.clear();
			audioNames.clear();
			audioIDs.reserve(resourceManager.getAudios().size());
			audioNames.reserve(resourceManager.getAudios().size());

			for (auto const& kv : resourceManager.getAudios()) {
				audioIDs.push_back(kv.first);
				audioNames.push_back(kv.second.getKey());
			}
			if (bgmIndex >= (int)audioNames.size()) bgmIndex = 0;
			if (sfxIndex >= (int)audioNames.size()) sfxIndex = 0;
		}
	}

	if (isPlayMode) {
		ImGui::EndDisabled();
	}

	if (!audioNames.empty()) {
		if (bgmIndex >= (int)audioNames.size()) bgmIndex = 0;
		ImGui::Text("Simple Audio Test:");
		if (ImGui::BeginCombo("Select Audio", audioNames[bgmIndex].c_str())) {
			for (int i = 0; i < (int)audioNames.size(); ++i) {
				bool selected = (i == bgmIndex);
				if (ImGui::Selectable(audioNames[i].c_str(), selected)) {
					bgmIndex = i;
				}
				if (selected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ResourceID bgmRID = audioIDs[bgmIndex];
		if (ImGui::Button("Play Audio")) {
			// create persistent instance and store mapping
			auto it = activeInstances.find(bgmRID);
			std::optional<AudioInstanceID> opt;
			if (it != activeInstances.end()) {
				audioSystem.resumeSound(it->second);
			}
			else {
				// check if audio type is music or sfx
				if (audioNames[bgmIndex].find("BGM_") != std::string::npos) {
					// create bgm instance
					AudioData::IndividualAudioData d{};
					d.resourceID = bgmRID;
					d.name = audioNames[bgmIndex];
					d.loop = true;
					d.group = AudioData::IndividualAudioData::Group::Music;
					d.volume = audioSystem.getBGMVolume();
					opt = audioSystem.createAudioInstance(d, INVALID_ENTITY_ID);
				}
				else if (audioNames[bgmIndex].find("SFX_") != std::string::npos) {
					// play sfx instance (one-shot)
					audioSystem.playSFX(bgmRID);
				}

				if (opt) {
					activeInstances[bgmRID] = *opt;
					audioSystem.resumeSound(*opt); // createAudioInstance starts paused
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Pause Audio")) {
			ResourceID rid = bgmRID;
			auto it = activeInstances.find(rid);
			if (it != activeInstances.end()) audioSystem.pauseSound(it->second);
		}
	}

	ImGui::Separator();
	ImGui::Spacing();

	//Show current game objects in scene
	if (ImGui::CollapsingHeader("Scene Objects", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Scene* scene = gsm.getCurrentScene();
		if (!scene) // if no scene, show message
		{
			ImGui::TextDisabled("No active scene loaded.");
		}
		else
		{
			bool isPlayMode = (m_editorMode == EditorMode::PLAY);
			if (isPlayMode) {
				ImGui::BeginDisabled();
			}

			for (size_t i = 0; i < scene->items.size(); ++i)
			{
				//const DrawItem& item = scene->items[i];
				//bool selected = (selectedEntityType == i); // check if obj is current selected one

				//// labels obj as Entity if it has no name
				//std::string label = item.name.empty()
				//	? "Entity " + std::to_string(i)
				//	: item.name;

				//if (ImGui::Selectable(label.c_str(), selected)) // selectable list
				//	selectedEntityType = static_cast<unsigned int>(i);

				DrawItem& item = scene->items[i];
				bool selected = (selectedEntityType == i);

				std::string label = item.name.empty()
					? "Entity " + std::to_string(i)
					: item.name;

				if (ImGui::Selectable(label.c_str(), selected))
					selectedEntityType = static_cast<unsigned int>(i);

				//accept dropped textures here
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE"))
					{
						IM_ASSERT(payload->DataSize == sizeof(GLuint));
						GLuint droppedTex = *reinterpret_cast<const GLuint*>(payload->Data);

						item.tex = droppedTex;
						logger.log("[ImGui] Applied texture via drag-drop to object: " + label);
					}
					ImGui::EndDragDropTarget();
				}
			}

			if (isPlayMode) {
				ImGui::EndDisabled();
			}
		}
	}

	// Add and remove buttons 
	Scene* sceneMut = gsm.getCurrentScene();

	if (sceneMut)
	{
		ImGui::Separator();

		//disable add/remove in play mode
		bool isPlayMode = (m_editorMode == EditorMode::PLAY);

		if (isPlayMode) {
			ImGui::BeginDisabled();
		}

		//this button will change to load a gameobject,, currently loads just an empty game object
		if (ImGui::Button("Add New Object"))
		{
			//save state for undo
			pushUndoState("Add New Object");

			// Create a simple new DrawItem
			DrawItem newItem;
			newItem.name = "NewObject_" + std::to_string(sceneMut->items.size());
			newItem.w = 0.1f;     // default width
			newItem.h = 0.1f;     // default height
			newItem.x = 0.5f;     // start centered
			newItem.y = 0.5f;
			newItem.angle = 0.0f; // no rotation

			// Create a matching ECS entity for this object
			ECS& ecs = gsm.getECS();
			EntityID e = ecs.createEntity();
			newItem.entityID = e;

			//default collider settings
			newItem.hasCollider = true;
			newItem.colOffsetX = 0.0f;
			newItem.colOffsetY = 0.0f;
			newItem.colW = newItem.w;
			newItem.colH = newItem.h;

			// Add it to the current scene
			sceneMut->items.push_back(newItem);
			selectedEntityType = static_cast<unsigned int>(sceneMut->items.size() - 1);

			logger.log("[ImGui] Added new object: " + newItem.name);
		}

		ImGui::SameLine();

		if (ImGui::Button("Remove Selected"))
		{
			if (!sceneMut->items.empty() && selectedEntityType < sceneMut->items.size())
			{
				//save state for undo
				pushUndoState("Remove Selected");

				logger.log("[ImGui] Removed object: " + sceneMut->items[selectedEntityType].name);
				sceneMut->items.erase(sceneMut->items.begin() + selectedEntityType);

				// Adjust selection index to stay valid
				if (selectedEntityType >= sceneMut->items.size() && !sceneMut->items.empty())
					selectedEntityType = static_cast<unsigned int>(sceneMut->items.size() - 1);
				else if (sceneMut->items.empty())
					selectedEntityType = 0;
			}
		}

		if (isPlayMode) {
			ImGui::EndDisabled();
		}

	}
	else
	{
		ImGui::TextDisabled("Scene not editable (no active level).");
	}

	ImGui::End();
}

void ImGuiManager::showTransformUI(/* A reference to a vector of object list objectList */) {
	// Fixed size for transform window
	/*constexpr float width = 300.0f;
	constexpr float height = 200.0f;*/

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Position the overlay at the top-right corner of the window, below entity list overlay
	/*ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y + 550.0f };
	ImGui::SetNextWindowPos(windowTopRightPos);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });*/
	/*ImGui::Begin("Transform", nullptr, windowFlags);
	ImGui::Text("Transform Controls");*/

	ImGui::Begin("Inspector", nullptr, windowFlags);
	ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "In-Game Property Editor"); // section title
	ImGui::Separator();

	Scene* scene = gsm.getCurrentScene();
	if (!scene || scene->items.empty()) {
		ImGui::TextDisabled("No scene or objects available."); // message shows when theres nothing in current scene
		ImGui::End();
		return;
	}

	if (selectedEntityType >= scene->items.size()) {
		ImGui::TextDisabled("No object selected.");
		ImGui::End();
		return;
	}

	DrawItem& item = scene->items[selectedEntityType];

	ImGui::Text("Editing: %s", item.name.empty() ? "(Unnamed)" : item.name.c_str());
	ImGui::Separator();

	//disable editing in play mode
	bool isPlayMode = (m_editorMode == EditorMode::PLAY);
	if (isPlayMode) {
		ImGui::BeginDisabled();
	}


	// edits the game objects actual transform
	if (ImGui::DragFloat("X", &item.x, 0.01f, 0.0f, 1.0f)) {
		if (ImGui::IsItemActivated()) {
			pushUndoState("Inspector transform change");
		}
	}

	// Y
	if (ImGui::DragFloat("Y", &item.y, 0.01f, 0.0f, 1.0f)) {
		if (ImGui::IsItemActivated()) {
			pushUndoState("Inspector transform change");
		}
	}

	// Width
	if (ImGui::DragFloat("Width", &item.w, 0.01f, 0.0f, 2.0f)) {
		if (ImGui::IsItemActivated()) {
			pushUndoState("Inspector transform change");
		}
	}

	// Height
	if (ImGui::DragFloat("Height", &item.h, 0.01f, 0.0f, 2.0f)) {
		if (ImGui::IsItemActivated()) {
			pushUndoState("Inspector transform change");
		}
	}

	// Angle
	if (ImGui::DragFloat("Angle", &item.angle, 0.5f, -360.0f, 360.0f)) {
		if (ImGui::IsItemActivated()) {
			pushUndoState("Inspector transform change");
		}
	}

	if (isPlayMode) {
		ImGui::EndDisabled();
	}

	//collision controls
	ImGui::Separator();
	ImGui::Text("Collision");

	ImGui::Checkbox("Has Collider", &item.hasCollider);

	if (item.hasCollider)
	{
		ImGui::DragFloat("Collider Offset X", &item.colOffsetX, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat("Collider Offset Y", &item.colOffsetY, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat("Collider Width", &item.colW, 0.01f, 0.0f, 2.0f);
		ImGui::DragFloat("Collider Height", &item.colH, 0.01f, 0.0f, 2.0f);

		if (ImGui::Button("Reset Collider to Sprite")) {
			item.colOffsetX = item.colOffsetY = 0.0f;
			item.colW = item.w;
			item.colH = item.h;
		}
	}

	/*if (changed)
	{
		logger.log("[ImGui] Updated transform of " + item.name);
	}*/
	/*
	// @Todo (axewhy): Change this with the selected entity transform. This is only a placeholder
	static Vector2D position(0.f, 0.f);
	static Matrix<3> rotationMatrix = Matrix<3>().Identity();
	static Matrix<3> scaleMatrix = Matrix<3>().Identity();


	// Position controls
	ImGui::SliderFloat("Position X", &position.x(), -1000.0f, 1000.0f);
	ImGui::SliderFloat("Position Y", &position.y(), -1000.0f, 1000.0f);

	// Scaling controls
	ImGui::SliderFloat("Scale X", &scaleMatrix[0][0], 0.1f, 10.0f);
	ImGui::SliderFloat("Scale Y", &scaleMatrix[1][1], 0.1f, 10.0f);

	// Rotation controls (in degrees)
	static float rotationDegrees = 0.0f;
	ImGui::SliderFloat("Rotation", &rotationDegrees, -180.0f, 180.0f);
	*/

	// Showing current game objects in scene
	/*if (ImGui::CollapsingHeader("Scene Object Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Scene* scene = gsm.getCurrentScene();
		if (scene && selectedEntityType < scene->items.size())
		{
			DrawItem& item = scene->items[selectedEntityType];
			ImGui::Text("Editing: %s", item.name.empty() ? "(Unnamed)" : item.name.c_str());
			ImGui::Separator();

			ImGui::DragFloat("X", &item.x, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Y", &item.y, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Width", &item.w, 0.01f, 0.0f, 2.0f);
			ImGui::DragFloat("Height", &item.h, 0.01f, 0.0f, 2.0f);
			ImGui::DragFloat("Angle", &item.angle, 0.5f, -360.0f, 360.0f);
		}
		else
		{
			ImGui::TextDisabled("No object selected or invalid index.");
		}
	}*/

	// --- Audio for this object -------------------------------------------------
	ImGui::Separator();
	if (ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ECS& ecs = gsm.getECS();

		// Get the ECS entity linked 
		EntityID eid = item.entityID;

		// If entityID is invalid, create automatically
		if (eid == INVALID_ENTITY_ID)
		{
			eid = ecs.createEntity();
			item.entityID = eid;
			logger.log("[Inspector] Auto-created ECS entity for " + item.name);
		}

		// Ensure the entity has an AudioData component
		AudioData* ad = ecs.getComponent<AudioData>(eid);
		if (!ad)
		{
			ImGui::TextDisabled("No AudioData on this object.");
			if (ImGui::Button("Add AudioData component"))
			{
				ecs.addComponent<AudioData>(eid, AudioData{});
				ad = ecs.getComponent<AudioData>(eid);
				logger.log("[Inspector] Added AudioData to entity " + std::to_string(eid));
			}
		}

		if (!ad)
		{
			// Could not add / still null
			ImGui::Separator();
			ImGui::End();
			return;
		}

		// Build the audio resource list from ResourceManager
		std::vector<ResourceID> audioIDs;
		std::vector<std::string> audioNames;
		audioIDs.reserve(resourceManager.getAudios().size());
		audioNames.reserve(resourceManager.getAudios().size());

		for (auto const& kv : resourceManager.getAudios()) {
			audioIDs.push_back(kv.first);
			audioNames.push_back(kv.second.getKey());
		}

		if (audioNames.empty()) {
			ImGui::TextDisabled("No audio assets loaded.");
		}
		else {
			static int newTrackIndex = 0;
			if (newTrackIndex >= (int)audioNames.size())
				newTrackIndex = 0;

			ImGui::Text("Add audio track to this object:");
			if (ImGui::BeginCombo("Audio Asset", audioNames[newTrackIndex].c_str())) {
				for (int i = 0; i < (int)audioNames.size(); ++i) {
					bool sel = (i == newTrackIndex);
					if (ImGui::Selectable(audioNames[i].c_str(), sel))
						newTrackIndex = i;
					if (sel) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button("Attach Selected Audio")) {
				ResourceID rid = audioIDs[newTrackIndex];

				AudioData::IndividualAudioData track{};
				track.resourceID = rid;
				track.name = audioNames[newTrackIndex];
				track.loop = false;
				track.group = AudioData::IndividualAudioData::Group::SFX; // or Music
				track.volume = 1.0f;
				track.pitch = 1.0f;

				ad->audios.push_back(track);

				logger.log("[Inspector] Attached audio '" + track.name +
					"' to object " + item.name);
			}
		}

		ImGui::Separator();
		ImGui::Text("Tracks on this object:");

		if (ad->audios.empty()) {
			ImGui::TextDisabled("No tracks attached.");
		}
		else {
			for (std::size_t i = 0; i < ad->audios.size(); ++i) {
				auto& track = ad->audios[i];
				ImGui::PushID((int)i);

				ImGui::Text("%s (RID:%u)",
					track.name.empty() ? "<unnamed>" : track.name.c_str(),
					track.resourceID);

				bool loopFlag = track.loop;
				ImGui::Checkbox("Loop", &loopFlag);
				track.loop = loopFlag;

				// Instance lookup for playback
				AudioInstanceID instanceID = 0;
				bool hasInstance = false;
				auto it = activeInstances.find(track.resourceID);
				if (it != activeInstances.end()) {
					instanceID = it->second;
					hasInstance = true;
				}

				if (ImGui::Button(hasInstance ? "Pause" : "Play")) {
					if (hasInstance) {
						audioSystem.pauseSound(instanceID);
					}
					else {
						auto opt = audioSystem.createAudioInstance(track, eid);
						if (opt) {
							activeInstances[track.resourceID] = *opt;
							audioSystem.resumeSound(*opt);
						}
						else {
							logger.error("[Inspector] Failed to create instance for RID " +
								std::to_string(track.resourceID));
						}
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Resume") && hasInstance) {
					audioSystem.resumeSound(instanceID);
				}
				if (ImGui::Button("Restart")) {
					auto it = activeInstances.find(track.resourceID);
					if (it != activeInstances.end()) {
						audioSystem.restartSound(it->second);   // reset audio to start
						audioSystem.resumeSound(it->second);    // play it again
					}
				}


				float currentVol = hasInstance ? audioSystem.getVolume(instanceID)
					: track.volume;
				if (ImGui::SliderFloat("Volume", &currentVol, 0.0f, 1.0f)) {
					if (hasInstance)
						audioSystem.modifyVolume(instanceID, currentVol);
					track.volume = currentVol;
				}

				float pitchVal = track.pitch;
				if (ImGui::DragFloat("Pitch", &pitchVal, 0.01f, 0.25f, 4.0f, "%.2f")) {
					track.pitch = pitchVal;
					// Apply runtime pitch
				}

				if (ImGui::Button("Remove Track"))
				{
					// If this track has an active instance, stop/pause it first
					auto itInst = activeInstances.find(track.resourceID);
					if (itInst != activeInstances.end()) {
						audioSystem.pauseSound(itInst->second);
						activeInstances.erase(itInst);
					}

					// Now remove the track data from this object
					ad->audios.erase(ad->audios.begin() + static_cast<std::ptrdiff_t>(i));

					ImGui::PopID();
					break;      // vector changed, break out of the loop
				}


				ImGui::Separator();
				ImGui::PopID();
			}
		}
	}


	ImGui::End();
}

void ImGuiManager::showLogsOverlayUI() {
	// Fixed size for log window
	constexpr float width = 600.0f;
	constexpr float height = 200.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize;
	//ImGuiWindowFlags_AlwaysVerticalScrollbar;

	// Position the overlay at the bottom-center of the window
	/*ImVec2 initialPos = { (windowTopLeft.x + windowBottomRight.x) / 2, windowBottomRight.y };
	ImGui::SetNextWindowPos(initialPos, ImGuiCond_Always, ImVec2(0.5f, 1.0f));
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });*/

	ImGui::Text("Logs");

	for (auto const& log : logger.getLogs()) {
		if (log.find("[ERROR]") != std::string::npos)
			ImGui::TextColored(ImVec4(255 / 255.f, 130 / 255.f, 130 / 255.f, 1.f), log.c_str());
		else if (log.find("[WARNING]") != std::string::npos)
			ImGui::TextColored(ImVec4(255 / 255.f, 255 / 255.f, 130 / 255.f, 1.f), log.c_str());
		else if (log.find("[LOG]") != std::string::npos)
			ImGui::TextColored(ImVec4(33 / 255.f, 165 / 255.f, 57 / 255.f, 1.f), log.c_str());
		else
			ImGui::TextUnformatted(log.c_str());
	}

	// Auto scroll to bottom
	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		ImGui::SetScrollHereY(1.0f);

}

void ImGuiManager::showAssetsBrowserUI() {
	ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Assets Browser");
	ImGui::Separator();

	bool isPlayMode = (m_editorMode == EditorMode::PLAY);

	// Small helper to list files in a folder (used by fonts/levels/prefabs/scenes/shaders)
	auto drawFolderFileList = [&](const char* debugName,
		const char* folderPath,
		int& selectedIdx)
		{
			ImGui::Text("%s (%s)", debugName, folderPath);
			ImGui::Separator();

			std::vector<std::string> files;
			if (std::filesystem::exists(folderPath)) {
				for (auto const& entry : std::filesystem::directory_iterator(folderPath)) {
					if (!entry.is_regular_file()) continue;
					files.push_back(entry.path().filename().string());
				}
			}

			if (files.empty()) {
				ImGui::TextDisabled("No files found in this folder.");
				return;
			}

			if (selectedIdx < 0 || selectedIdx >= (int)files.size())
				selectedIdx = 0;

			ImVec2 listSize(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 10.0f);
			std::string id = "##";
			id += debugName;
			id += "List";

			if (ImGui::BeginListBox(id.c_str(), listSize)) {
				for (int i = 0; i < (int)files.size(); ++i) {
					bool isSelected = (i == selectedIdx);
					if (ImGui::Selectable(files[i].c_str(), isSelected)) {
						selectedIdx = i;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}
		};

	if (ImGui::BeginTabBar("AssetTypeTabs"))
	{
		// ============================================================
		//  TEXTURES
		// ============================================================
		if (ImGui::BeginTabItem("Textures"))
		{
			// === SECTION 1: Import Controls (DISABLED IN PLAY MODE) ===
			if (isPlayMode) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
				ImGui::Text("Asset import disabled during play mode");
				ImGui::PopStyleColor();
				ImGui::Separator();
				ImGui::BeginDisabled();
			}

			ImGui::Text("Import New Texture:");
			static char newPath[256] = "assets/textures/";
			ImGui::InputText("New Texture Path", newPath, IM_ARRAYSIZE(newPath));
			ImGui::Spacing();

			if (ImGui::Button("Import Texture"))
			{
				std::string sourcePath = newPath;

				if (sourcePath.empty())
				{
					logger.error("[ImGui] No source path specified!");
				}
				else
				{
					try
					{
						std::filesystem::path destFolder = "assets/textures/";
						std::filesystem::create_directories(destFolder);

						std::filesystem::path src(sourcePath);
						std::string filename = src.filename().string();
						std::filesystem::path destPath = destFolder / filename;

						std::filesystem::copy_file(src, destPath,
							std::filesystem::copy_options::overwrite_existing);

						std::string texKey = destPath.stem().string();
						resourceManager.loadTexture(texKey, destPath.string());
						resourceManager.getGLTextureFromFile(destPath.string());

						logger.log("[ImGui] Imported new texture: " + destPath.string());
					}
					catch (const std::exception& e)
					{
						logger.error(std::string("[ImGui] Failed to import texture: ") + e.what());
					}
				}

				newPath[0] = '\0';
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::Button("Rescan assets/textures")) {
				std::size_t added = resourceManager.reloadTexturesFromDirectory("assets/textures");
				logger.log("[ImGui] Rescan complete. New textures added: " + std::to_string(added));
			}
			ImGui::SameLine();

			if (ImGui::Button("Clear GL texture cache")) {
				resourceManager.clearAllGLTexturesCache();
			}

			if (isPlayMode) {
				ImGui::EndDisabled();  // END SECTION 1
			}

			// === SECTION 2: Texture Display ===
			ImGui::Text("Loaded Textures:");
			ImGui::Separator();

			static float thumbnailSize = 60.0f;
			float panelWidth = ImGui::GetContentRegionAvail().x;
			float padding = 10.0f;
			float cellSize = thumbnailSize + padding;
			int columnCount = (int)(panelWidth / cellSize);
			if (columnCount < 1) columnCount = 1;

			if (ImGui::BeginTable("TextureTable", columnCount, ImGuiTableFlags_SizingStretchSame))
			{
				auto& textures = resourceManager.getTextures();
				int index = 0;

				if (isPlayMode) {
					ImGui::BeginDisabled();
				}

				for (auto& [id, texFile] : textures)
				{
					const std::string& texName = texFile.getKey();
					const std::string& texPath = texFile.getFilePath();

					GLuint texID = resourceManager.getGLTextureFromFile(texPath);
					if (texID == 0) continue;

					ImGui::TableNextColumn();
					ImGui::PushID(index++);

					//thumbnail: click still directly applies to currently selected object
					if (ImGui::ImageButton(
						"##thumb",
						(ImTextureID)(intptr_t)texID,
						ImVec2(thumbnailSize, thumbnailSize)))
					{
						Scene* scene = gsm.getCurrentScene();
						if (scene && selectedEntityType < scene->items.size())
						{
							scene->items[selectedEntityType].tex = texID;
							logger.log("[ImGui] Applied texture: " + texName);
						}
					}

					//drag source: drag this thumbnail onto entity
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
					{
						ImGui::SetDragDropPayload("ASSET_TEXTURE", &texID, sizeof(GLuint));
						ImGui::Text("Texture: %s", texName.c_str());
						ImGui::EndDragDropSource();
					}

					ImGui::TextWrapped(texName.c_str());
					ImGui::PopID();
				}

				if (isPlayMode) {
					ImGui::EndDisabled();
				}

				ImGui::EndTable();
			}

			ImGui::EndTabItem();
		}

		// ============================================================
		//  AUDIO
		// ============================================================
		if (ImGui::BeginTabItem("Audio"))
		{
			ImGui::Text("Audio Assets:");
			ImGui::Separator();

			std::vector<ResourceID> audioIDs;
			std::vector<std::string> audioNames;
			audioIDs.reserve(resourceManager.getAudios().size());
			audioNames.reserve(resourceManager.getAudios().size());

			for (auto const& kv : resourceManager.getAudios()) {
				audioIDs.push_back(kv.first);
				audioNames.push_back(kv.second.getKey());
			}

			static int selectedAudioIndex = 0;

			if (isPlayMode) {
				ImGui::BeginDisabled();
			}

			if (ImGui::Button("Reload audio assets")) {
				auto added = resourceManager.reloadAudiosFromDirectory("assets/audio");
				if (added > 0) {
					audioIDs.clear();
					audioNames.clear();
					audioIDs.reserve(resourceManager.getAudios().size());
					audioNames.reserve(resourceManager.getAudios().size());
					for (auto const& kv : resourceManager.getAudios()) {
						audioIDs.push_back(kv.first);
						audioNames.push_back(kv.second.getKey());
					}
					if (selectedAudioIndex >= (int)audioNames.size())
						selectedAudioIndex = 0;
				}
			}

			if (isPlayMode) {
				ImGui::EndDisabled();
			}

			if (!audioNames.empty()) {
				if (selectedAudioIndex >= (int)audioNames.size())
					selectedAudioIndex = 0;

				ImVec2 listSize(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 10.0f);
				if (ImGui::BeginListBox("##AudioList", listSize)) {
					for (int i = 0; i < (int)audioNames.size(); ++i) {
						bool isSelected = (i == selectedAudioIndex);
						if (ImGui::Selectable(audioNames[i].c_str(), isSelected)) {
							selectedAudioIndex = i;
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndListBox();
				}

				ResourceID rid = audioIDs[selectedAudioIndex];

				ImGui::Separator();
				ImGui::Text("Selected: %s", audioNames[selectedAudioIndex].c_str());

				if (ImGui::Button("Play")) {
					auto it = activeInstances.find(rid);
					std::optional<AudioInstanceID> opt;

					if (it != activeInstances.end()) {
						audioSystem.resumeSound(it->second);
					}
					else {
						const std::string& name = audioNames[selectedAudioIndex];
						if (name.find("BGM_") != std::string::npos) {
							AudioData::IndividualAudioData data{};
							data.resourceID = rid;
							data.name = name;
							data.loop = true;
							data.group = AudioData::IndividualAudioData::Group::Music;
							data.volume = audioSystem.getBGMVolume();
							opt = audioSystem.createAudioInstance(data, INVALID_ENTITY_ID);
						}
						else if (name.find("SFX_") != std::string::npos) {
							audioSystem.playSFX(rid);
						}

						if (opt) {
							activeInstances[rid] = *opt;
							audioSystem.resumeSound(*opt);
						}
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("Pause")) {
					auto it = activeInstances.find(rid);
					if (it != activeInstances.end()) {
						audioSystem.pauseSound(it->second);
						activeInstances.erase(it);
					}
				}


				ImGui::SameLine();
				if (ImGui::Button("Stop")) {
					auto it = activeInstances.find(rid);
					if (it != activeInstances.end()) {
						//audioSystem.stopSound(it->second);
						activeInstances.erase(it);
					}
				}
			}
			else {
				ImGui::TextDisabled("No audio assets loaded.");
			}

			ImGui::EndTabItem();
		}

		// ============================================================
		//  FONTS / LEVELS / PREFABS / SCENES / SHADERS
		//  (directory listing only for now)
		// ============================================================
		if (ImGui::BeginTabItem("Fonts")) {
			static int fontIdx = -1;
			drawFolderFileList("Fonts", "assets/fonts", fontIdx);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Levels")) {
			static int levelIdx = -1;
			drawFolderFileList("Levels", "assets/levels", levelIdx);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Prefabs")) {
			static int prefabIdx = -1;
			drawFolderFileList("Prefabs", "assets/prefabs", prefabIdx);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Scenes")) {
			static int sceneIdx = -1;
			drawFolderFileList("Scenes", "assets/scenes", sceneIdx);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Shaders")) {
			static int shaderIdx = -1;
			drawFolderFileList("Shaders", "assets/shaders", shaderIdx);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}


void ImGuiManager::showBottomDockUI()
{
	/*
	//float dockPadding = 8.0f;   // gap between viewport and dock
	float dockHeight = 200.0f; // desired dock height
	float rightPanelWidth = 300.0f; // width of right sidebar

	// dock should begin directly below the viewport
	float dockYPos = vpMaxWin.y ;

	// dock should stop before the right sidebar begins
	float dockXPos = windowTopLeft.x + 8.0f;
	float dockWidth = (windowBottomRight.x - rightPanelWidth) - dockXPos;

	// doesnt exceed the window bottom
	if (dockYPos + dockHeight > windowBottomRight.y)
		dockYPos = windowBottomRight.y - dockHeight - 4.0f;

	ImVec2 dockPos = { dockXPos, dockYPos };
	ImVec2 dockSize = { dockWidth, dockHeight };

	ImGui::SetNextWindowPos(dockPos);
	ImGui::SetNextWindowSize(dockSize);*/

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysVerticalScrollbar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));

	ImGui::Begin("Editor Dock", nullptr, flags);

	if (ImGui::BeginTabBar("DockTabs"))
	{
		if (ImGui::BeginTabItem("Assets Browser"))
		{
			showAssetsBrowserUI();  // no Begin/End inside
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Logs"))
		{
			showLogsOverlayUI();    // no Begin/End inside
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Animation Editor"))
		{
			showAnimationEditorUI();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Camera")) {
			Cam2D_Params p = Cam2D_GetActiveParams();
			bool changed = false;

			changed |= ImGui::SliderFloat("Center X", &p.cx, 0.0f, 1.0f);
			changed |= ImGui::SliderFloat("Center Y", &p.cy, 0.0f, 1.0f);
			changed |= ImGui::SliderFloat("Zoom", &p.zoom, 0.25f, 6.0f);
			changed |= ImGui::SliderFloat("MinZoom", &p.minZoom, 0.25f, 6.0f);
			changed |= ImGui::SliderFloat("MaxZoom", &p.maxZoom, 0.25f, 6.0f);

			if (p.minZoom > p.maxZoom) std::swap(p.minZoom, p.maxZoom);

			if (changed) {
				Cam2D_SetActiveParams(p);
			}

			ImGui::EndTabItem();
		}


		ImGui::EndTabBar();
	}

	ImGui::End();
	ImGui::PopStyleVar(2);
}

void ImGuiManager::showAnimationEditorUI() {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;
	constexpr float width = 400.0f, height = 330.0f;
	ImVec2 pos{ windowBottomRight.x - width - 780.0f, windowTopLeft.y + 520.0f };
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize({ width, height });
	ImGui::Text("Sprite Animation Editor");

	ECS& ecs = gsm.getECS();

	// Build list of anim-capable entities
	static int selected = -1;
	std::vector<EntityID> animEntities;
	for (EntityID e : ecs.getAllEntities()) {
		if (ecs.getComponent<SpriteAnimationComponent>(e) && ecs.getComponent<ImageRenderer>(e))
			animEntities.push_back(e);
	}

	if (animEntities.empty()) {
		ImGui::TextDisabled("No entities with SpriteAnimationComponent.");
		return;
	}

	// Entity selector
	if (selected < 0 || selected >= (int)animEntities.size()) selected = 0;
	if (ImGui::BeginCombo("Entity", std::to_string(animEntities[selected]).c_str())) {
		for (int i = 0; i < (int)animEntities.size(); ++i) {
			bool sel = (i == selected);
			if (ImGui::Selectable(std::to_string(animEntities[i]).c_str(), sel))
				selected = i;
			if (sel) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	EntityID eid = animEntities[selected];
	auto* anim = ecs.getComponent<SpriteAnimationComponent>(eid);
	auto* img = ecs.getComponent<ImageRenderer>(eid);

	bool isPlayMode = (m_editorMode == EditorMode::PLAY);
	if (isPlayMode) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
		ImGui::Text("Animation editing disabled during play mode");
		ImGui::PopStyleColor();
		ImGui::Separator();
		ImGui::BeginDisabled();
	}

	// Current animation
	if (anim->currentAnim.empty() && !anim->animations.empty())
		anim->currentAnim = anim->animations.begin()->first;

	// Animation list
	ImGui::Text("Animations:");
	if (ImGui::BeginListBox("##animlist", ImVec2(-FLT_MIN, 100))) {
		for (auto& kv : anim->animations) {
			bool sel = (kv.first == anim->currentAnim);
			if (ImGui::Selectable(kv.first.c_str(), sel))
				anim->currentAnim = kv.first;
			if (sel) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}

	// Edit selected animation
	if (!anim->currentAnim.empty()) {
		auto& def = anim->animations[anim->currentAnim];
		int frames = def.frames;
		float frameTime = def.frameTime;
		bool loop = def.loop;

		ImGui::DragInt("Frames", &frames, 0.2f, 1, 1024);
		ImGui::DragFloat("Frame Time", &frameTime, 0.001f, 0.001f, 5.0f, "%.3f");
		ImGui::Checkbox("Loop", &loop);

		if (ImGui::Button("Apply Changes")) {
			def.frames = frames;
			def.frameTime = frameTime;
			def.loop = loop;
			anim->currentFrame = 0;
			anim->timer = 0.0f;
			anim->playing = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete Animation") && anim->animations.size() > 1) {
			anim->animations.erase(anim->currentAnim);
			if (anim->animations.empty()) anim->currentAnim.clear();
			else anim->currentAnim = anim->animations.begin()->first;
			anim->currentFrame = 0;
			anim->timer = 0.0f;
		}
	}

	ImGui::Separator();
	// Add new animation
	static char newName[64] = "";
	static int newFrames = 1;
	static float newFrameTime = 0.1f;
	static bool newLoop = true;
	static char newSpritePath[256] = "assets/textures/";
	static int newCols = 1;
	static int newRows = 1;

	ImGui::InputText("New Name", newName, IM_ARRAYSIZE(newName));
	ImGui::InputText("New Sprite Path", newSpritePath, IM_ARRAYSIZE(newSpritePath));
	ImGui::DragInt("New Frames", &newFrames, 0.2f, 1, 1024);
	ImGui::DragFloat("New Frame Time", &newFrameTime, 0.001f, 0.001f, 5.0f, "%.3f");
	ImGui::Checkbox("New Loop", &newLoop);
	ImGui::DragInt("New Columns", &newCols, 0.2f, 1, 256);
	ImGui::DragInt("New Rows", &newRows, 0.2f, 1, 256);

	if (ImGui::Button("Add Animation") && newName[0]) {
		SpriteAnimationComponent::Animation a{};
		a.frames = newFrames;
		a.frameTime = newFrameTime;
		a.loop = newLoop;
		a.sheetColumns = newCols;
		a.sheetRows = newRows;
		if (newSpritePath[0]) {
			std::string p = newSpritePath;
			if (p.rfind("assets/", 0) != 0) p = "assets/" + p;
			a.spritePath = p;
		}
		anim->animations[newName] = a;
		anim->currentAnim = newName;
		anim->currentFrame = 0;
		anim->timer = 0.0f;
		anim->playing = true;
		newName[0] = '\0';
	}

	// Playback controls
	ImGui::Separator();
	ImGui::Text("Playback:");
	if (ImGui::Button(anim->playing ? "Pause" : "Play"))
		anim->playing = !anim->playing;
	ImGui::SameLine();
	if (ImGui::Button("Restart")) {
		anim->currentFrame = 0;
		anim->timer = 0.0f;
		anim->playing = true;
	}

	if (isPlayMode) {
		ImGui::EndDisabled();
	}
}

void ImGuiManager::createDockspace() {
	// Setup fullscreen invisible window for docking
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	// Make window fill entire screen
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	// Remove window decorations
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
	windowFlags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	// Create the invisible fullscreen window
	ImGui::Begin("DockSpace", nullptr, windowFlags);
	ImGui::PopStyleVar(3);

	// CREATE THE ACTUAL DOCKSPACE
	ImGuiID dockspaceID = ImGui::GetID("NocturnalStudioDockspace");
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	// setup initial laayout
	setupInitialDockingLayout();

	// Menu bar
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Exit")) {
				// Handle exit
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			// Add toggles for showing/hiding panels
			if (ImGui::MenuItem("Viewport")) {}
			if (ImGui::MenuItem("Performance")) {}
			if (ImGui::MenuItem("Entity Picker")) {}
			if (ImGui::MenuItem("Inspector")) {}
			if (ImGui::MenuItem("Editor Dock")) {}

			ImGui::Separator();

			// ⭐ ADD RESET LAYOUT OPTION
			if (ImGui::MenuItem("Reset Layout")) {
				resetDockingLayout();
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();

		// Calculate center position
		float menuBarWidth = ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX();
		float buttonWidth = 100.0f;
		float centerPos = (menuBarWidth * 0.5f) - (buttonWidth * 0.5f);
		ImGui::SetCursorPosX(centerPos);

		if (m_editorMode == EditorMode::EDIT) {
			// Green Play button
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.4f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.2f, 1.0f));
			if (ImGui::Button("Play", ImVec2(buttonWidth, 0))) {
				onPlayButtonPressed();
			}
			ImGui::PopStyleColor(3);
		}
		else {
			// Red Stop button
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
			if (ImGui::Button("Stop", ImVec2(buttonWidth, 0))) {
				onStopButtonPressed();
			}
			ImGui::PopStyleColor(3);
		}

		// Show current mode
		ImGui::Separator();
		ImGui::Text("Mode:");
		ImGui::SameLine();
		if (m_editorMode == EditorMode::EDIT) {
			ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.3f, 1.0f), "EDIT");
		}
		else {
			ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "PLAY");
		}
	}

	ImGui::EndMenuBar();


	ImGui::End();
}

void ImGuiManager::setupInitialDockingLayout() {
	// wait a couple frames so dockspace exists
	static int frameCount = 0;
	if (frameCount < 2) {
		++frameCount;
		return;
	}

	if (m_dockingLayoutInitialized)
		return;

	ImGuiID dockspace = ImGui::GetID("NocturnalStudioDockspace");

	// fresh dockspace
	ImGui::DockBuilderRemoveNode(dockspace);
	ImGui::DockBuilderAddNode(dockspace, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspace, ImGui::GetMainViewport()->Size);

	ImGuiID dock_main = dockspace;
	ImGuiID dock_bottom = 0;
	ImGuiID dock_left = 0;
	ImGuiID dock_right = 0;

	// 1) Split full dockspace into TOP and BOTTOM (bottom = full-width editor dock)
	dock_bottom = ImGui::DockBuilderSplitNode(
		dock_main,          // node to split
		ImGuiDir_Down,
		0.25f,              // 25% height for bottom
		nullptr,            // return id for bottom
		&dock_main          // dock_main becomes the TOP area
	);

	// 2) Split TOP into LEFT (Entity Picker) and remaining
	dock_left = ImGui::DockBuilderSplitNode(
		dock_main,
		ImGuiDir_Left,
		0.22f,              // 22% width for left
		nullptr,
		&dock_main          // dock_main is now the center strip
	);

	// 3) Split remaining TOP into RIGHT (Inspector) and CENTER (Viewport)
	dock_right = ImGui::DockBuilderSplitNode(
		dock_main,
		ImGuiDir_Right,
		0.25f,              // 25% width for right
		nullptr,
		&dock_main          // dock_main is now the center area
	);
	ImGuiID dock_center = dock_main;

	// ---- Dock windows ----
	ImGui::DockBuilderDockWindow("Viewport", dock_center);
	ImGui::DockBuilderDockWindow("Entity Picker", dock_left);
	ImGui::DockBuilderDockWindow("Inspector", dock_right);
	ImGui::DockBuilderDockWindow("Overlay", dock_right);
	ImGui::DockBuilderDockWindow("Performance", dock_right);
	ImGui::DockBuilderDockWindow("Editor Dock", dock_bottom);

	ImGui::DockBuilderFinish(dockspace);

	m_dockingLayoutInitialized = true;
	logger.log("Initial docking layout set.");
}


void ImGuiManager::resetDockingLayout() {
	ImGuiID dockspace = ImGui::GetID("NocturnalStudioDockspace");
	ImGui::DockBuilderRemoveNode(dockspace);
	m_dockingLayoutInitialized = false;
	logger.log("Docking layout reset");
}

void ImGuiManager::saveSceneSnapshot() {
	Scene* scene = gsm.getCurrentScene();
	if (!scene) {
		logger.warning("No scene to save snapshot");
		return;
	}

	// Create JSON string
	m_savedSceneState = "[";

	for (size_t i = 0; i < scene->items.size(); ++i) {
		const DrawItem& item = scene->items[i];

		m_savedSceneState += "{";
		m_savedSceneState += "\"name\":\"" + item.name + "\",";
		m_savedSceneState += "\"x\":" + std::to_string(item.x) + ",";
		m_savedSceneState += "\"y\":" + std::to_string(item.y) + ",";
		m_savedSceneState += "\"w\":" + std::to_string(item.w) + ",";
		m_savedSceneState += "\"h\":" + std::to_string(item.h) + ",";
		m_savedSceneState += "\"angle\":" + std::to_string(item.angle) + ",";
		m_savedSceneState += "\"tex\":" + std::to_string(item.tex) + ",";

		// CRITICAL: Save animal-specific properties
		m_savedSceneState += "\"item_type\":\"" + item.item_type + "\",";
		m_savedSceneState += "\"health_bar\":" + std::to_string(item.health_bar) + ",";
		m_savedSceneState += "\"hunger_bar\":" + std::to_string(item.hunger_bar) + ",";
		m_savedSceneState += "\"sanity_bar\":" + std::to_string(item.sanity_bar) + ",";
		m_savedSceneState += "\"still_alive\":" + std::string(item.still_alive ? "true" : "false") + ",";
		m_savedSceneState += "\"is_draggable\":" + std::string(item.is_draggable ? "true" : "false") + ",";
		m_savedSceneState += "\"is_dragging\":" + std::string(item.is_dragging ? "true" : "false") + ",";
		m_savedSceneState += "\"acc_timer\":" + std::to_string(item.acc_timer) + ",";
		m_savedSceneState += "\"staring_countdown\":" + std::to_string(item.staring_countdown);

		m_savedSceneState += "}";

		if (i < scene->items.size() - 1) m_savedSceneState += ",";
	}

	m_savedSceneState += "]";

	logger.log("Scene snapshot saved (" + std::to_string(scene->items.size()) + " items) with full animal state");
}

void ImGuiManager::restoreSceneSnapshot() {
	if (m_savedSceneState.empty()) {
		logger.warning("No snapshot to restore");
		return;
	}

	Scene* scene = gsm.getCurrentScene();
	if (!scene) return;

	// store original data temporarily
	std::vector<DrawItem> originalItems = scene->items;

	// clear current items
	scene->items.clear();

	// Parse the saved JSON (simplified parsing)
	// NOTE: This is a basic implementation. For production, use a proper JSON library
	size_t pos = 0;
	std::string json = m_savedSceneState;

	while ((pos = json.find("{", pos)) != std::string::npos) {
		size_t endPos = json.find("}", pos);
		if (endPos == std::string::npos) break;

		std::string objStr = json.substr(pos, endPos - pos + 1);
		DrawItem item{};

		// Explicitly set critical values to known defaults
		item.health_bar = 0.0f;
		item.hunger_bar = 0.0f;
		item.sanity_bar = 0.0f;
		item.acc_timer = 0.0f;
		item.staring_countdown = 10.0f;

		// Parse fields (simplified)
		/*
		auto getValue = [](const std::string& str, const std::string& key) -> std::string {
			size_t keyPos = str.find("\"" + key + "\":");
			if (keyPos == std::string::npos) return "";

			keyPos = str.find(":", keyPos) + 1;
			size_t endPos = str.find_first_of(",}", keyPos);
			std::string value = str.substr(keyPos, endPos - keyPos);

			// Remove quotes if present
			if (value.front() == '"') value = value.substr(1, value.length() - 2);

			return value;
			};
		*/
		auto getValue = [](const std::string& str, const std::string& key) -> std::string {
			size_t keyPos = str.find("\"" + key + "\":");
			if (keyPos == std::string::npos) return "";

			keyPos = str.find(":", keyPos) + 1;

			// Skip whitespace
			while (keyPos < str.length() && (str[keyPos] == ' ' || str[keyPos] == '\t')) {
				keyPos++;
			}

			size_t endPos = str.find_first_of(",}", keyPos);
			if (endPos == std::string::npos) return "";  // ← ADDED: Safety check

			std::string value = str.substr(keyPos, endPos - keyPos);

			// Trim whitespace from end
			while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
				value.pop_back();
			}

			// Remove quotes if present (with safety check)
			if (!value.empty() && value.length() >= 2 &&  // ← ADDED: Safety checks
				value.front() == '"' && value.back() == '"') {
				value = value.substr(1, value.length() - 2);
			}

			return value;
			};

		item.name = getValue(objStr, "name");
		item.x = std::stof(getValue(objStr, "x"));
		item.y = std::stof(getValue(objStr, "y"));
		item.w = std::stof(getValue(objStr, "w"));
		item.h = std::stof(getValue(objStr, "h"));
		item.angle = std::stof(getValue(objStr, "angle"));
		item.tex = std::stoul(getValue(objStr, "tex"));

		// CRITICAL: Restore animal-specific properties
		std::string itemType = getValue(objStr, "item_type");
		if (!itemType.empty()) {
			item.item_type = itemType;
		}

		std::string healthStr = getValue(objStr, "health_bar");
		if (!healthStr.empty()) {
			item.health_bar = std::stof(healthStr);
		}

		std::string hungerStr = getValue(objStr, "hunger_bar");
		if (!hungerStr.empty()) {
			item.hunger_bar = std::stof(hungerStr);
		}

		std::string sanityStr = getValue(objStr, "sanity_bar");
		if (!sanityStr.empty()) {
			item.sanity_bar = std::stof(sanityStr);
		}

		std::string aliveStr = getValue(objStr, "still_alive");
		if (!aliveStr.empty()) {
			item.still_alive = (aliveStr == "true" || aliveStr == "1");
		}

		std::string draggableStr = getValue(objStr, "is_draggable");
		if (!draggableStr.empty()) {
			item.is_draggable = (draggableStr == "true" || draggableStr == "1");
		}

		std::string draggingStr = getValue(objStr, "is_dragging");
		if (!draggingStr.empty()) {
			item.is_dragging = (draggingStr == "true" || draggingStr == "1");
		}

		std::string accTimerStr = getValue(objStr, "acc_timer");
		if (!accTimerStr.empty()) {
			item.acc_timer = std::stof(accTimerStr);
		}

		std::string staringStr = getValue(objStr, "staring_countdown");
		if (!staringStr.empty()) {
			item.staring_countdown = std::stof(staringStr);
		}

		scene->items.push_back(item);
		pos = endPos + 1;
	}

	logger.log("Scene snapshot restored (" + std::to_string(scene->items.size()) + " items) with full animal state");
	m_savedSceneState.clear();
}

void ImGuiManager::onPlayButtonPressed() {
	if (m_editorMode == EditorMode::PLAY) return;

	logger.log("=== ENTERING PLAY MODE ===");

	// Save initial state ONLY on the first play
	if (!m_hasInitialState) {
		saveSceneSnapshot();
		m_initialSceneState = m_savedSceneState;  // Store as initial
		m_hasInitialState = true;
		logger.log("Initial scene state stored");
	}

	// Switch to play mode
	m_editorMode = EditorMode::PLAY;

	logger.log("Play mode started");
}

void ImGuiManager::onStopButtonPressed() {
	if (m_editorMode == EditorMode::EDIT) return;

	logger.log("=== EXITING PLAY MODE ===");

	// Get current scene and reset all animal values to defaults
	Scene* scene = gsm.getCurrentScene();
	if (scene) {
		for (auto& item : scene->items) {
			if (item.item_type == "animal") {
				// Reset bars to full
				item.health_bar = 100.0f;
				item.hunger_bar = 100.0f;
				item.sanity_bar = 100.0f;

				// Reset timers
				item.acc_timer = 0.0f;
				item.staring_countdown = 10.0f;

				// Reset status
				item.still_alive = true;
				item.is_dragging = false;
			}
		}
	}

	// Switch back to the main menu scene
	gsm.requestScene("MainMenu");

	// Set app state to Menu (locks number keys, enables menu buttons)
	gsm.goToMenu();

	// Switch back to edit mode
	m_editorMode = EditorMode::EDIT;

	logger.log("Edit mode restored - returned to menu");
}

void ImGuiManager::draw_logic_selector() {
	//ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
	//	ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	//ImGuiViewport* viewport = ImGui::GetMainViewport();
	//ImVec2 viewport_size = viewport->WorkSize;
	//ImVec2 viewport_pos = viewport->WorkPos;

	//// Make sure the window has a reasonable size
	//float inspector_width = 100.0f;
	//float window_height = 300.0f; // Ensure this is not too small

	//ImVec2 window_pos = ImVec2(viewport_pos.x + viewport_size.x - inspector_width, viewport_pos.y);
	//ImVec2 window_size = ImVec2(inspector_width, window_height);

	//ImGui::SetNextWindowPos(window_pos);
	//ImGui::SetNextWindowSize(window_size);

	//// Add these flags to ensure proper rendering
	//ImGui::Begin("Logic Selector", nullptr, windowFlags);

	//// ADD ACTUAL CONTENT - this might be why it's black
	//ImGui::Text("Logic Selector Content");
	//ImGui::Button("Test Button");
	// Add more UI elements here


	//ImGui::Text("Object: %s", selected_obj->name.c_str());

	//// Add logic dropdown
	//static const char* logic_types[] = { "Flamingo", "Owl", "Horse", "Seal" };
	//static int current_logic = 0;

	//ImGui::Combo("Add Logic", &current_logic, logic_types, IM_ARRAYSIZE(logic_types));
	//if (ImGui::Button("Add Selected Logic")) {
	//	selected_obj->add_logic(logic_types[current_logic]);
	//}

	//// Show current logics
	//ImGui::Separator();
	//ImGui::Text("Current Logics:");

	//for (int i = 0; i < selected_obj->logics.size(); i++) {
	//	ImGui::PushID(i);
	//	auto& logic = selected_obj->logics[i];

	//	ImGui::Checkbox("", &logic->enabled);
	//	ImGui::SameLine();
	//	ImGui::Text("%s", logic->logic_name.c_str());
	//	ImGui::SameLine();
	//	if (ImGui::Button("Remove")) {
	//		selected_obj->remove_logic(i);
	//		ImGui::PopID();
	//		break;
	//	}

	//	ImGui::PopID();
	//}

	//ImGui::End();
}

void ImGuiManager::pushUndoState(const char* reason)
{
	if (m_editorMode == EditorMode::PLAY)
		return; //no undo recording in play mode

	Scene* scene = gsm.getCurrentScene();
	if (!scene)
		return;

	//keep up to kMaxUndoSteps snapshots
	if (m_undoStack.size() >= kMaxUndoSteps) {
		//drop the oldest
		m_undoStack.erase(m_undoStack.begin());
	}

	m_undoStack.emplace_back(scene->items);

	if (reason) {
		logger.log(std::string("[Undo] Saved state: ") + reason);
	}
}

void ImGuiManager::undoLastAction()
{
	Scene* scene = gsm.getCurrentScene();
	if (!scene)
		return;

	//need at least 2 states, previous and current
	if (m_undoStack.size() < 2) {
		logger.warning("[Undo] No more undo steps available.");
		return;
	}

	//remove current state
	m_undoStack.pop_back();

	//restore previous
	scene->items = m_undoStack.back();

	//fix selection index
	if (selectedEntityType >= scene->items.size()) {
		selectedEntityType = scene->items.empty()
			? 0
			: static_cast<unsigned int>(scene->items.size() - 1);
	}

	logger.log("[Undo] Undo applied.");
}


void ImGuiManager::handleViewportPicking(const ImVec2& imgMin,
	const ImVec2& imgMax,
	int vpW, int vpH)
{
	//only allow editing in EDIT mode
	if (m_editorMode == EditorMode::PLAY)
		return;

	Scene* scene = gsm.getCurrentScene();
	if (!scene)
		return;

	auto& items = scene->items;
	if (items.empty())
		return;

	//mouse position (screen space)
	ImVec2 mouse = ImGui::GetMousePos();

	//is mouse inside the viewport image
	bool inside =
		mouse.x >= imgMin.x && mouse.x <= imgMax.x &&
		mouse.y >= imgMin.y && mouse.y <= imgMax.y;

	//mouse in normalized [0..1] viewport coordinates
	float localX = 0.0f;
	float localY = 0.0f;

	if (inside) {
		float px = mouse.x - imgMin.x;
		float py = mouse.y - imgMin.y;

		localX = px / static_cast<float>(vpW);
		localY = py / static_cast<float>(vpH);
	}

	//--------delete currently selected item--------
	if (ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
		if (!items.empty() && selectedEntityType < items.size()) {
			//save state for undo
			pushUndoState("Delete object (Delete key)");
			logger.log("[ImGui] Deleted object via Delete key: " + items[selectedEntityType].name);
			items.erase(items.begin() + static_cast<std::ptrdiff_t>(selectedEntityType));
			if (selectedEntityType >= items.size() && !items.empty())
				selectedEntityType = static_cast<unsigned int>(items.size() - 1);
			else if (items.empty())
				selectedEntityType = 0;
		}
	}

	//if mouse left the viewport, stop dragging
	if (!inside) {
		if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			m_isDragging = false;
			m_draggedItem = -1;
		}
		m_hoveredItem = -1;
		return;
	}

	//--------hit-test items (from top-most to bottom)--------
	int hitIndex = -1;

	for (int i = static_cast<int>(items.size()) - 1; i >= 0; --i) {
		const DrawItem& it = items[static_cast<size_t>(i)];

		float minX, maxX, minY, maxY;

		if (it.hasCollider && it.colW > 0.0f && it.colH > 0.0f) {
			// collider is also top-left + size in normalized space
			minX = it.x + it.colOffsetX;
			minY = it.y + it.colOffsetY;
			maxX = minX + it.colW;
			maxY = minY + it.colH;
		}
		else {
			// sprite rect: top-left (x,y) with width/height
			minX = it.x;
			minY = it.y;
			maxX = it.x + it.w;
			maxY = it.y + it.h;
		}

		if (localX >= minX && localX <= maxX &&
			localY >= minY && localY <= maxY)
		{
			hitIndex = i;
			break;
		}
	}



	m_hoveredItem = hitIndex;

	//--------click to select/start dragging--------
	if (hitIndex >= 0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		//save pre-drag state
		pushUndoState("Begin viewport drag/select");
		selectedEntityType = static_cast<unsigned int>(hitIndex);
		m_draggedItem = hitIndex;
		m_isDragging = true;

		DrawItem& it = items[static_cast<size_t>(hitIndex)];
		m_dragOffset.x = localX - it.x;
		m_dragOffset.y = localY - it.y;

		logger.log("[ImGui] Selected item from viewport: " + it.name);
	}

	//--------drag while LMB is held--------
	if (m_isDragging && m_draggedItem >= 0 &&
		ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		if (m_draggedItem < static_cast<int>(items.size())) {
			DrawItem& it = items[static_cast<size_t>(m_draggedItem)];

			float newX = localX - m_dragOffset.x;
			float newY = localY - m_dragOffset.y;

			//keep inside world bounds
			newX = std::clamp(newX, 0.0f, 1.0f);
			newY = std::clamp(newY, 0.0f, 1.0f);

			it.x = newX;
			it.y = newY;
		}
		else {
			//dragged item was removed
			m_isDragging = false;
			m_draggedItem = -1;
		}
	}
	//--------stop dragging when LMB is released--------
	if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		m_isDragging = false;
		m_draggedItem = -1;
	}
}

void ImGuiManager::handleExternalFileDrops()
{
	//get & clear any file paths dropped onto the GLFW window
	auto files = mainWindow.consumeDroppedFiles();
	if (files.empty())
		return;

	logger.log("[Drop] handleExternalFileDrops: got " + std::to_string(files.size()) + " file(s)");

	for (const std::string& src : files)
	{
		try {
			std::filesystem::path srcPath(src);

			//only support images for now
			std::string ext = srcPath.extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
			if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".bmp")
			{
				logger.warning("[Drop] Ignored non-image file: " + src);
				continue;
			}

			// Copy into assets/textures
			std::filesystem::path destFolder = "assets/textures";
			std::filesystem::create_directories(destFolder);

			std::filesystem::path dest = destFolder / srcPath.filename();
			std::filesystem::copy_file(
				srcPath, dest,
				std::filesystem::copy_options::overwrite_existing
			);

			//register with ResourceManager so it appears in the thumbnail list
			std::string key = dest.stem().string();
			std::string fullPath = dest.string();

			resourceManager.loadTexture(key, fullPath);
			resourceManager.getGLTextureFromFile(fullPath); // create GL texture

			logger.log("[Drop] Imported texture via OS drag-drop: " + fullPath);
		}
		catch (const std::exception& e)
		{
			logger.error(std::string("[Drop] Failed to import dropped file: ") + e.what());
		}
	}
}
