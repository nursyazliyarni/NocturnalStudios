/*!------------------------------------------------------------------------------------------------
	File: ImGUIManager.cpp
	Project: NocturnalStudio

	This file contains the implementation of the ImGuiManager class, which manages the integration
	of the ImGui library into the customize game engine. It handles the initialization,
	frame updates, and rendering of ImGui components, as well as providing various UI elements
	for debugging and interaction within the engine.

	Primary Author:
		Lau Xiang Yang (100%)
		- Created the ImGuiManager class and its specializations.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "../include/core/ImGuiManager.h"

void Gui::updateImGuiManager(double& deltaTime, ImGuiManager& imGuiManager) {
	imGuiManager.updateDeltaTime(deltaTime);

	// Get main viewport position and size
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	imGuiManager.updateWindowTopLeft(mainViewport->Pos);
	imGuiManager.updateWindowBottomRight({ mainViewport->Pos.x + mainViewport->Size.x, mainViewport->Pos.y + mainViewport->Size.y });

	imGuiManager.startFrame();
	imGuiManager.showViewportWindowUI();
	imGuiManager.showMousePosOverlayUI();
	imGuiManager.showPerformanceOverlayUI();
	imGuiManager.showEntityListUI();
	imGuiManager.showTransformUI();
	imGuiManager.showLogsOverlayUI();
	imGuiManager.endFrame();
}

ImGuiManager::ImGuiManager(Window& w, Renderer& r, Texture& tex, Input& i) : mainWindow{ w }, renderer{ r }, texture{ tex }, input{i}, windowTopLeft {}, windowBottomRight{}, deltaTime{ 0. }, selectedEntityType{} {

	// Init ImGui context
	ImGui::CreateContext();

	// Setup renderer bindings
	// Set to false->ImGui will not install its own callbacks
	// My Input class will handle ALL input callbacks
	ImGui_ImplGlfw_InitForOpenGL(mainWindow.handle(), false);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Setup style
	ImGui::StyleColorsClassic();
	ImGuiIO& io = ImGui::GetIO();

	// Logging
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

void ImGuiManager::showViewportWindowUI() {
	constexpr float overlayWidthOffset = 300.0f;
	constexpr float overlayHeightOffset = 200.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize;

	// Calculate the size of the viewport window
	float viewportWidth = windowBottomRight.x - windowTopLeft.x - overlayWidthOffset;
	float viewportHeight = windowBottomRight.y - windowTopLeft.y - overlayHeightOffset;

	// Position the viewport window at the top-left corner of the main window
	ImGui::SetNextWindowPos(windowTopLeft);
	ImGui::SetNextWindowSize(ImVec2(viewportWidth, viewportHeight));
	ImGui::Begin("Viewport", nullptr, windowFlags);

	// ?? ADD: actual drawable region (excludes title/padding)
	const ImVec2 avail = ImGui::GetContentRegionAvail();
	const int vw = (int)std::max(1.0f, avail.x);
	const int vh = (int)std::max(1.0f, avail.y);

	// Initialize and bind FBO for rendering
	// renderer.initFBO(viewportHeight, viewportWidth);            // REMOVE this (args swapped)
	renderer.initFBO(vw, vh);                                      // ADD: correct (width, height)
	renderer.bindFBO();

	// ?? ADD: set viewport + orthographic to pixel-space for the FBO
	renderer.resize(vw, vh);

	// You can keep your clear (fine for 2D)
	renderer.clear(0.1f, 0.1f, 0.12f);

	// @Todo (axewhy): Render the actual viewport content here, e.g., using a framebuffer texture
	// Render background to FBO
	renderer.background_render(texture.get_textureID(0), "bg_texture");

	// ?? ADD: draw all SHAPES from Renderer.cpp (keeps ImGui clean of GL)
	renderer.drawViewportShapes(vw, vh);

	renderer.unbindFBO();

	// show FBO in the panel (use the content size so it matches exactly)
	// ImGui::Image((void*)(intptr_t)renderer.getFBOTexture(), ImVec2(viewportWidth, viewportHeight));
	// NEW: flip V when sampling the FBO in ImGui
	ImGui::Image((ImTextureID)(intptr_t)renderer.getFBOTexture(),
		ImVec2(viewportWidth, viewportHeight),
		ImVec2(0, 1), ImVec2(1, 0));


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
	constexpr double secondsToMs = 1000.0;
	constexpr float width = 300.0f;
	constexpr float height = 50.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Position the overlay at the top-right corner of the window, below mouse position overlay
	ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y + 100.0f };
	ImGui::SetNextWindowPos(windowTopRightPos);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });

	ImGui::Begin("Performance", nullptr, windowFlags);
	ImGui::Text("Current FPS: %.2f %.2lfms", 1 / deltaTime, deltaTime * secondsToMs);

	// @Todo (axewhy): Add more performance metrics like the percentage of usage between different systems in the game engine

	ImGui::End();
}

void ImGuiManager::showEntityListUI(/* Maybe an enum of paramEntityType, Maybe a Reference to a std::vector of objectList?*/) {
	// Fixed size for entity list window
	constexpr float width = 300.0f;
	constexpr float height = 100.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Position the overlay at the top-right corner of the window, below performance overlay
	ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y + 150.0f };
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

	ImGui::End();
}

void ImGuiManager::showTransformUI(/* A reference to a vector of object list objectList */) {
	// Fixed size for transform window
	constexpr float width = 300.0f;
	constexpr float height = 200.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Position the overlay at the top-right corner of the window, below entity list overlay
	ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y + 250.0f };
	ImGui::SetNextWindowPos(windowTopRightPos);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });
	ImGui::Begin("Transform", nullptr, windowFlags);
	ImGui::Text("Transform Controls");

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
