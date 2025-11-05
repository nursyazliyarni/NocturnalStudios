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
#include "../include/graphics/Window.h"
#include "../include/graphics/Renderer.h"
#include "../include/graphics/Texture.h"
#include "../include/input/Input.h"
#include "../include/physics/Physics.h"
#include "../System/Audio_System.h"
#include "../resources/ResourceManager.h"

#include <vector>
#include <unordered_map>
#include "GameManager.h"


extern Logger logger;

class ImGuiManager {
public:
	// Accept lists of resource IDs for quick audio testing.
	ImGuiManager(Window& window, Renderer& renderer,  Input& input, 
		Physics& physics, AudioSystem& audio, ResourceManager& resourceManager, GameManager& gsm);
	~ImGuiManager();

	void updateDeltaTime(double& dt);
	void getDeltaTime(double& dt) const;
	void updateWindowTopLeft(const ImVec2& topLeft);
	void updateWindowBottomRight(const ImVec2& bottomRight);
	void setSystemTimings(double imGuiTime, double input);
	
	// temp setter for render time since renderer is called in showViewportWindowUI
	void setRenderTime(double time);
	void setPhysicTime(double time);
	void setInputTime(double time);

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

	void getViewportRectWindow(ImVec2& outMin, ImVec2& outMax) const { outMin = vpMinWin; outMax = vpMaxWin; }

	void showAssetsBrowserUI();
	void showBottomDockUI();

private:
	Window& mainWindow;
	Renderer& renderer;
	//Texture& texture;
	Input& input;
	Physics& physics;
	AudioSystem& audioSystem;
	ResourceManager& resourceManager;
	GameManager& gsm;

	ImVec2 windowTopLeft;
	ImVec2 windowBottomRight;

	double deltaTime;
	double imGuiTime;
	double inputTime;
	double logicTime;
	double renderTime;
	double physicsTime;


	unsigned int selectedEntityType;

	// Track the active instance for each resource (if created)
	std::unordered_map<ResourceID, AudioInstanceID> activeInstances;

	ImVec2 vpMinWin{}, vpMaxWin{};
};


namespace Gui {
	// Non Member functions
	void updateImGuiManager(double& deltaTime, ImGuiManager& imGuiManager);
}
#endif // ImGuiManager_h