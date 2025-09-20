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

	void updateDeltaTime(double& dt);
	void getDeltaTime(double& dt) const;

	void startFrame();
	void endFrame();

	Vector2D getWindowTopLeft() const;
	Vector2D getWindowBottomRight() const;

	void showMousePosOverlayUI();
	void showPerformanceOverlayUI();
	void showLogsOverlayUI();
	void showEntityListUI(/* An enum of entityType type, Maybe a reference to someArrayOrVectorType entityList */);
	void showTransformUI(/* A reference to a vector of objects objectList */);

private:
	GLFWwindow* window;

	ImVec2 windowTopLeft;
	ImVec2 windowBottomRight;

	double deltaTime;

	unsigned int selectedEntityType;
};


namespace Gui {
	// Non Member functions
	void updateImGuiManager(double& deltaTime, ImGuiManager& imGuiManager);
}
#endif // ImGuiManager_h