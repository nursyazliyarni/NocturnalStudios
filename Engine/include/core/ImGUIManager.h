#ifndef ImGuiManager_h
#define ImGuiManager_h

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/backends/imgui_impl_glfw.h"
#include "../imgui/backends/imgui_impl_opengl3.h"

#include "../include/math/Vector.h"
#include "../include/math/Matrix.h"
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
	void showEntityListUI(/* An enum of entityType type, Maybe a reference to someArrayOrVectorType entityList */);
	void showTransformUI(/* A reference to a vector of objects objectList */);

private:
	ImVec2 windowTopLeft;
	ImVec2 windowBottomRight;

	bool showOverlay;

	unsigned int selectedEntityID = 0;
};

#endif // ImGuiManager_h