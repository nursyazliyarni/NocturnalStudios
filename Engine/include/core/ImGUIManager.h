/*!------------------------------------------------------------------------------------------------
	File: ImGUIManager.h
	Project: NocturnalStudio

	This file contains the declaration of the ImGuiManager class, which manages the integration
	of the ImGui library into the customize game engine. It handles the initialization,
	frame updates, and rendering of ImGui components, as well as providing various UI elements
	for debugging and interaction within the engine.

	Primary Author:
		Lau Xiang Yang (100%)
		- Created the ImGuiManager class and its specializations.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

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
	void updateWindowTopLeft(const ImVec2& topLeft);
	void updateWindowBottomRight(const ImVec2& bottomRight);

	void startFrame();
	void endFrame();

	Vector2D getWindowTopLeft() const;
	Vector2D getWindowBottomRight() const;

	void showViewportWindowUI();
	void showMousePosOverlayUI();
	void showLogsOverlayUI();

	// @Todo(axewhy): These functions are not fully implemented yet
	void showPerformanceOverlayUI();
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