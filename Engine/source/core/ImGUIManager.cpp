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

	// Logging
	logger.log("ImGui initialized successfully.");
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

void ImGuiManager::showMousePosOverlayUI() {
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
	
	// Get main viewport position and size
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	windowTopLeft = mainViewport->Pos;
	windowBottomRight = { mainViewport->Pos.x + mainViewport->Size.x, mainViewport->Pos.y + mainViewport->Size.y };

	// Position the overlay at the top-right corner of the window
	ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y };
	ImGui::SetNextWindowPos(windowTopRightPos);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });

	ImGui::Begin("Overlay", nullptr, windowFlags);
	ImGui::Text("Mouse position");
	ImGui::Text("World pos: %.2f, %.2f", worldPosition.x(), worldPosition.y());
	ImGui::Text("View pos: %.2f, %.2f", viewPortPosition.x(), viewPortPosition.y());
	ImGui::Text("Screen pos: %.2f, %.2f", screenPosition.x(), screenPosition.y());
	ImGui::End();
}

void ImGuiManager::showPerformanceOverlayUI(double& dt) {
	constexpr double secondsToMs = 1000.0;
	constexpr float width = 200.0f;
	constexpr float height = 50.0f;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Position the overlay at the top-right corner of the window, below mouse position overlay
	ImVec2 windowTopRightPos = { windowBottomRight.x - width, windowTopLeft.y + 100.0f };
	ImGui::SetNextWindowPos(windowTopRightPos);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });

	ImGui::Begin("Performance", nullptr, windowFlags);
	ImGui::Text("Current FPS: %.2f %.2lfms", 1 / dt, dt * secondsToMs);

	ImGui::End();
}

void ImGuiManager::showLogsOverlayUI() {
	// Fixed size for log window
	constexpr float width = 600.0f;
	constexpr float height = 200.0f;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoSavedSettings;

	// Position the overlay at the bottom-center of the window
    ImVec2 initialPos = { (windowTopLeft.x + windowBottomRight.x) / 2, windowBottomRight.y };
    ImGui::SetNextWindowPos(initialPos, ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize({ width, height });

    ImGui::Begin("Logs", nullptr, windowFlags);

	// Add a Child window to enable vertical and horizontal scrolling
	ImGui::BeginChild("LogRegionHorizontal", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	// test log message
	logger.log("This is a repeated message");
	logger.warning("This is a repeated warning message");
	logger.error("This is a repeated long log message to test the wrapping functionality of the log window. It will wrap around and be fully visible within the window.");

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

	ImGui::EndChild();
    ImGui::End();
}
