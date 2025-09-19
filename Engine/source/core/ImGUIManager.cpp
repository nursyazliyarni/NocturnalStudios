#include "../include/core/ImGuiManager.h"

ImGuiManager::ImGuiManager(GLFWwindow*& window) : windowTopLeft{}, windowBottomRight{}, showOverlay{ true } {

	// Init ImGui context
	ImGui::CreateContext();

	// Setup renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Setup style
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();

}

ImGuiManager::~ImGuiManager() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
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

void ImGuiManager::showOverlayUI() {
	if (!showOverlay) return;

	constexpr float width = 200.0f;
	constexpr float height = 100.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Get mouse position in different coordinate spaces
	// @Todo (axewhy): Implement these when input system is ready
	Vector2D worldPosition;
	Vector2D viewPortPosition;
	Vector2D screenPosition;

	//ImVec2 windowTopRight = { windowBottomRight.x - width, windowTopLeft.y };
	//ImGui::SetNextWindowPos(windowTopRight);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });

	ImGui::Begin("Overlay", nullptr, windowFlags);
	ImGui::Text("Mouse position");
	ImGui::Text("World pos: %.2f, %.2f", worldPosition.x(), worldPosition.y());
	ImGui::Text("View pos: %.2f, %.2f", viewPortPosition.x(), viewPortPosition.y());
	ImGui::Text("Screen pos: %.2f, %.2f", screenPosition.x(), screenPosition.y());
	ImGui::End();

}


