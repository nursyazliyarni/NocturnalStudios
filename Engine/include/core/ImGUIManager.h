#ifndef ImGuiManager_h
#define ImGuiManager_h

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/backends/imgui_impl_glfw.h"
#include "../imgui/backends/imgui_impl_opengl3.h"

#include "../include/math/Vector.h"
#include "../include/core/Logger.h"

extern Logger logger;

class ImGuiManager {
public:
	ImGuiManager(GLFWwindow*& window);
	~ImGuiManager();

	void startFrame();
	void endFrame();

	Vector2D getWindowTopLeft() const;
	Vector2D getWindowBottomRight() const;

	void showMousePosOverlayUI();
	void showPerformanceOverlayUI(double& dt);
	void showLogsOverlayUI();

private:
	ImVec2 windowTopLeft;
	ImVec2 windowBottomRight;

	bool showOverlay;
};

#endif // ImGuiManager_h