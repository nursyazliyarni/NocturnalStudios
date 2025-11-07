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


namespace {
	using Clock = std::chrono::high_resolution_clock;
}

void Gui::updateImGuiManager(double& deltaTime, ImGuiManager& imGuiManager) {
	// Measure the entire ImGui update using a utility and avoid manual start/end pairs
	double imGuiSec = TimeUtils::MeasureSeconds([&]() {
		imGuiManager.updateDeltaTime(deltaTime);

		// Get main viewport position and size
		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		imGuiManager.updateWindowTopLeft(mainViewport->Pos);
		imGuiManager.updateWindowBottomRight({ mainViewport->Pos.x + mainViewport->Size.x, mainViewport->Pos.y + mainViewport->Size.y });

		imGuiManager.startFrame();
		imGuiManager.showViewportWindowUI();
		//imGuiManager.showAssetsBrowserUI();
		imGuiManager.showMousePosOverlayUI();
		imGuiManager.showPerformanceOverlayUI();
		imGuiManager.showTransformUI();
		imGuiManager.showEntityListUI();
		//imGuiManager.showLogsOverlayUI();
		imGuiManager.showBottomDockUI();
		imGuiManager.endFrame();
	});

	// Preserve logic time measured within UI instead of resetting to
	imGuiManager.setSystemTimings(imGuiSec);
}

ImGuiManager::ImGuiManager(Window& w, Renderer& r,
	Input& i, Physics& p,
	AudioSystem& a, ResourceManager& rm,
	GameManager& g, Animal_Logic& l)
	: mainWindow{ w }, renderer{ r },
	input{ i }, physics{ p }, audioSystem{ a }, resourceManager{ rm },
	gsm{ g }, logic{l}, windowTopLeft {}, windowBottomRight{},
	deltaTime{ 0.0 }, imGuiTime{ 0.0 }, inputTime{ 0.0 },
	logicTime{ 0.0 }, renderTime{ 0.0 }, physicsTime{ 0.0 },
	selectedEntityType{ 0 }, activeInstances{}
{
	ImGui::CreateContext();
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

void ImGuiManager::getDeltaTime(double& dt) const {
	dt = deltaTime;
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
	constexpr float overlayW = 300.f, overlayH = 200.f;
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize;

	float vpW = windowBottomRight.x - windowTopLeft.x - overlayW;
	float vpH = windowBottomRight.y - windowTopLeft.y - overlayH;

	ImGui::SetNextWindowPos(windowTopLeft);
	ImGui::SetNextWindowSize(ImVec2(vpW, vpH));
	ImGui::Begin("Viewport", nullptr, flags);

	const ImVec2 avail = ImGui::GetContentRegionAvail();
	const int vw = (int)std::max(1.0f, avail.x);
	const int vh = (int)std::max(1.0f, avail.y);

	// Physics timing scope
	{
		TimeUtils::ScopedTimer t{ physicsTime };
		physics.update_sprite_movement(renderer, input);
		physics.update_sprite_scaling(renderer, input);
		physics.update_sprite_rotation(renderer, input);
	}

	// Render timing scope (includes scene render + debug draw + unbind)
	{
		TimeUtils::ScopedTimer t{ renderTime };
		renderer.initFBO(vw, vh);
		renderer.bindFBO();
		renderer.resize(vw, vh);
		renderer.clear(0.06f, 0.06f, 0.07f);

		// Draw current SCENE into the FBO
		gsm.render(renderer, vw, vh);

		// Text overlay (rendered into the viewport FBO)
		renderer.setTextProjection(vw, vh); // bottom-left origin for the text shader

		// Resolve fonts from ResourceManager (cache IDs across frames)
		static std::optional<ResourceID> s_fontA;
		static std::optional<ResourceID> s_fontB;
		if (!s_fontA) s_fontA = resourceManager.findResource<Font>("NanumBrushScript-Regular");
		if (!s_fontB) s_fontB = resourceManager.findResource<Font>("PAPYRUS");

		if (s_fontA) {
			const Font& f = resourceManager.getFont(*s_fontA);
			renderer.drawText(f, "Nocturnal Studio Engine",
							 25.0f, vh - 50.0f, // near top (BL origin)
							 1.0f, glm::vec3(1.0f, 0.0f, 0.2f));
		}
		if (s_fontB) {
			const Font& f2 = resourceManager.getFont(*s_fontB);
			renderer.drawText(f2, "Sample Font: Papyrus",
							 25.0f, vh - 100.0f, // near top (BL origin)
							 1.0f, glm::vec3(0.2f, 0.8f, 1.0f));
		}

		// Debugging Drawer //
		static bool printed = false;
		if (Debug::Drawer::Instance().IsActive()) {
			if (!printed) {
				std::cout << "[ImGuiManager] DebugDraw active\n";
				printed = true;
			}

			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glUseProgram(0);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			Debug::Drawer::Instance().Render(Matrix<3>().Identity());

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glPopAttrib();
		}
		else {
			printed = false;
		}
		// Debugging Drawer //

		renderer.unbindFBO();
	}

	// Show the FBO as an ImGui Image (flipped V)
	ImVec2 imgMin = ImGui::GetCursorScreenPos();
	ImGui::Image((ImTextureID)(intptr_t)renderer.getFBOTexture(),
	ImVec2(static_cast<float>(vw), static_cast<float>(vh)), ImVec2(0, 1), ImVec2(1, 0));
	ImVec2 imgMax = ImVec2(imgMin.x + vw, imgMin.y + vh);

	// Logic timing (animal state update)
	logicTime = TimeUtils::MeasureSeconds([&]() {
		logic.update_animal_ALLstate(gsm);
	});

	// Handle click -> normalized coords -> GSM
	// (Input handles clicks; store the rect in window client coords)
	vpMinWin = ImVec2(imgMin.x - windowTopLeft.x, imgMin.y - windowTopLeft.y);
	vpMaxWin = ImVec2(imgMax.x - windowTopLeft.x, imgMax.y - windowTopLeft.y);

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
	ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y };
	ImGui::SetNextWindowPos(windowTopRightPos);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });

	ImGui::Begin("Overlay", nullptr, windowFlags);
	ImGui::Text("Mouse position: %.2f, %.2f", mouse_pos[0], mouse_pos[1]);
	ImGui::Text("World pos: %.2f, %.2f", worldPosition.x(), worldPosition.y());
	ImGui::Text("View pos: %.2f, %.2f", viewPortPosition.x(), viewPortPosition.y());
	ImGui::Text("Screen pos: %.2f, %.2f", screenPosition.x(), screenPosition.y());
	ImGui::End();
}

void ImGuiManager::showPerformanceOverlayUI() {
	constexpr double secondsToMs =1000.0;
	constexpr float width = 300.0f;
	constexpr float height = 150.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Position the overlay at the top-right corner of the window, below mouse position overlay
	ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y + 100.0f };
	ImGui::SetNextWindowPos(windowTopRightPos);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });

	ImGui::Begin("Performance", nullptr, windowFlags);

	// Clamp dt to avoid div-by-zero and negative values
	double dt = deltaTime;
	if (dt <=0.0) dt =1e-6; // tiny epsilon when paused or first frame
	ImGui::Text("Current FPS: %.2f (%.2f ms)",1.0 / dt, dt * secondsToMs);

	// Compute percentages against the frame time (dt)
	auto pct = [dt](double t) -> double { return (t <=0.0) ?0.0 : (t / dt) *100.0; };

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
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Position the overlay at the top-right corner of the window, below performance overlay
	ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y + 250.0f };
	ImGui::SetNextWindowPos(windowTopRightPos);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });

	ImGui::Begin("Entity Picker", nullptr, windowFlags);
	ImGui::Text("Entity List");

	// if (selectedEntityID == paramEntityIDList::MAX) selectedEntityID = paramEntityIDList::MAX - 1; // we assume paramEntityIDList is an enum with it's last entry as MAX
	// e.g enum paramEntityIDList { ENTITY_A, ENTITY_B, ENTITY_C, MAX };

	// Picker controls
	ImGui::Text("Selected Entity Type: %d", selectedEntityType);
	ImGui::SameLine();
	if (ImGui::Button("-") && selectedEntityType > 0) selectedEntityType--;
	ImGui::SameLine();
	if (ImGui::Button("+")) selectedEntityType++;

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
				if (audioNames[bgmIndex].find("bgm_") != std::string::npos) {
					// create bgm instance
					AudioData::IndividualAudioData d{};
					d.resourceID = bgmRID;
					d.name = audioNames[bgmIndex];
					d.loop = true;
					d.group = AudioData::IndividualAudioData::Group::Music;
					d.volume = audioSystem.getBGMVolume();
					opt = audioSystem.createAudioInstance(d, INVALID_ENTITY_ID);
				}
				else if (audioNames[bgmIndex].find("sfx_") != std::string::npos) {
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
		const Scene* scene = gsm.getCurrentScene();
		if (!scene) // if no scene, show message
		{
			ImGui::TextDisabled("No active scene loaded.");
		}
		else
		{
			for (size_t i = 0; i < scene->items.size(); ++i)
			{
				const DrawItem& item = scene->items[i];
				bool selected = (selectedEntityType == i); // check if obj is current selected one

				// labels obj as Entity if it has no name
				std::string label = item.name.empty()
					? "Entity " + std::to_string(i)
					: item.name;

				if (ImGui::Selectable(label.c_str(), selected)) // selectable list
					selectedEntityType = static_cast<unsigned int>(i);
			}
		}
	}

	// Add and remove buttons 
	Scene* sceneMut = gsm.getCurrentScene();

	if (sceneMut)
	{
		ImGui::Separator();
		//this button will change to load a gameobject,, currently loads just an empty game object
		if (ImGui::Button("Add New Object")) 
		{
			// Create a simple new DrawItem
			DrawItem newItem;
			newItem.name = "NewObject_" + std::to_string(sceneMut->items.size());
			newItem.w = 0.1f;     // default width
			newItem.h = 0.1f;     // default height
			newItem.x = 0.5f;     // start centered
			newItem.y = 0.5f;
			newItem.angle = 0.0f; // no rotation

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
				logger.log("[ImGui] Removed object: " + sceneMut->items[selectedEntityType].name);
				sceneMut->items.erase(sceneMut->items.begin() + selectedEntityType);

				// Adjust selection index to stay valid
				if (selectedEntityType >= sceneMut->items.size() && !sceneMut->items.empty())
					selectedEntityType = static_cast<unsigned int>(sceneMut->items.size() - 1);
				else if (sceneMut->items.empty())
					selectedEntityType = 0;
			}
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
	constexpr float width = 300.0f;
	constexpr float height = 200.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Position the overlay at the top-right corner of the window, below entity list overlay
	ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y + 550.0f };
	ImGui::SetNextWindowPos(windowTopRightPos);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });
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

	// edits the game objects actual transform
	bool changed = false;
	changed |= ImGui::DragFloat("X", &item.x, 0.01f, 0.0f, 1.0f);
	changed |= ImGui::DragFloat("Y", &item.y, 0.01f, 0.0f, 1.0f);
	changed |= ImGui::DragFloat("Width", &item.w, 0.01f, 0.0f, 2.0f);
	changed |= ImGui::DragFloat("Height", &item.h, 0.01f, 0.0f, 2.0f);
	changed |= ImGui::DragFloat("Angle", &item.angle, 0.5f, -360.0f, 360.0f);

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

	ImGui::End();
}

void ImGuiManager::showLogsOverlayUI() {
	// Fixed size for log window
	constexpr float width = 600.0f;
	constexpr float height = 200.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize;

	// Position the overlay at the bottom-center of the window
    ImVec2 initialPos = { (windowTopLeft.x + windowBottomRight.x) / 2, windowBottomRight.y };
    ImGui::SetNextWindowPos(initialPos, ImGuiCond_Always, ImVec2(0.5f, 1.0f));
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });

	ImGui::Begin("Logs", nullptr, windowFlags);

	// Add a Child window to enable vertical and horizontal scrolling
	ImGui::BeginChild("LogRegionHorizontal", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

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

	ImGui::EndChild();
	ImGui::End();
}

void ImGuiManager::showAssetsBrowserUI() {
	constexpr float width = 400.0f;
	constexpr float height = 350.0f;
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

	ImVec2 pos = { windowBottomRight.x - width - 340.0f, windowTopLeft.y + 520.0f }; // near Transform window
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize({ width, height });

	ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Assets Browser");
	ImGui::Separator();
	
	// import new texture
	ImGui::Text("Import New Texture:");

	// input buffer (remembered each frame)
	static char newPath[256] = "assets/textures/";

	// text box for typing file path
	ImGui::InputText("New Texture Path", newPath, IM_ARRAYSIZE(newPath));

	// add a small vertical space
	ImGui::Spacing();

	// import button
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
				// ensure textures folder exists
				std::filesystem::path destFolder = "assets/textures/";
				std::filesystem::create_directories(destFolder);

				// get filename and destination path
				std::filesystem::path src(sourcePath);
				std::string filename = src.filename().string();
				std::filesystem::path destPath = destFolder / filename;

				// copy file into your project assets folder
				std::filesystem::copy_file(src, destPath,
					std::filesystem::copy_options::overwrite_existing);

				// register and load the new texture
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

		// clear the text field after importing
		newPath[0] = '\0';
	}
	
	ImGui::Text("Loaded Textures:");
	ImGui::Separator();

	// thumbnail size
	static float thumbnailSize = 60.0f;
	
	// thumbnail grid
	float panelWidth = ImGui::GetContentRegionAvail().x; // total space in tab


	float padding = 10.0f;
	float cellSize = thumbnailSize + padding;
	int columnCount = (int)(panelWidth / cellSize);
	if (columnCount < 1) columnCount = 1;
	
	// texture grid table 
	if (ImGui::BeginTable("TextureTable", columnCount, ImGuiTableFlags_SizingStretchSame))
	{
		auto& textures = resourceManager.getTextures();
		int index = 0;

		for (auto& [id, texFile] : textures)
		{
			const std::string& texName = texFile.getKey();
			const std::string& texPath = texFile.getFilePath();

			GLuint texID = resourceManager.getGLTextureFromFile(texPath);
			if (texID == 0) continue;

			ImGui::TableNextColumn();
			ImGui::PushID(index++);

			if (ImGui::ImageButton((ImTextureID)(intptr_t)texID, ImVec2(thumbnailSize, thumbnailSize)))
			{
				Scene* scene = gsm.getCurrentScene();
				if (scene && selectedEntityType < scene->items.size())
				{
					scene->items[selectedEntityType].tex = texID;
					logger.log("[ImGui] Applied texture: " + texName);
				}
			}

			ImGui::TextWrapped(texName.c_str());
			ImGui::PopID();
		}

		ImGui::EndTable();
	}
}

void ImGuiManager::showBottomDockUI()
{

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
	ImGui::SetNextWindowSize(dockSize);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize;

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