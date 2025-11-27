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
#include "logic/Animal_Logic.h"

#include <vector>
#include <unordered_map>
#include "GameManager.h"


extern Logger logger;

class ImGuiManager {
public:

	enum class EditorMode {
		EDIT,
		PLAY
	};

	// Accept lists of resource IDs for quick audio testing.
	ImGuiManager(Window& window, Renderer& renderer,  Input& input, 
		Physics& physics, AudioSystem& audio, ResourceManager& resourceManager, GameManager& gsm, Animal_Logic& logic);
	~ImGuiManager();

	void updateDeltaTime(double& dt);
	double getDeltaTime(double& dt) const;
	void updateWindowTopLeft(const ImVec2& topLeft);
	void updateWindowBottomRight(const ImVec2& bottomRight);
	void setSystemTimings(double imGuiTime);
	
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
	ImVec2 get_mousePOS(bool is_normalized);

	// @Todo(axewhy): These functions are not fully implemented yet
	void showPerformanceOverlayUI();
	void showEntityListUI(/* An enum of entityType type, Maybe a reference to someArrayOrVectorType entityList */);
	void showTransformUI(/* A reference to a vector of objects objectList */);

	void getViewportRectWindow(ImVec2& outMin, ImVec2& outMax) const { outMin = vpMinWin; outMax = vpMaxWin; }

	void showAssetsBrowserUI();
	void showBottomDockUI();
	void showAnimationEditorUI();   

	//dockspace
	void createDockspace();
	void setupInitialDockingLayout();  
	void resetDockingLayout();

	// Expose last measured logic time (seconds)
	double getLogicTime() const { return logicTime; }

	void onPlayButtonPressed();
	void onStopButtonPressed();
	bool isPlaying() const { return m_editorMode == EditorMode::PLAY; }

	void saveSceneSnapshot();
	void restoreSceneSnapshot();

private:
	bool m_showFPS = false;
	bool m_dockingLayoutInitialized = false;
	Window& mainWindow;
	Renderer& renderer;
	//Texture& texture;
	Input& input;
	Physics& physics;
	AudioSystem& audioSystem;
	ResourceManager& resourceManager;
	GameManager& gsm;
	Animal_Logic& logic;

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

	EditorMode m_editorMode = EditorMode::PLAY;
	std::string m_savedSceneState;

	int  m_hoveredItem = -1;
	int  m_draggedItem = -1; //index currently being dragged, or -1
	bool m_isDragging = false; //true while left mouse button held on an item
	ImVec2 m_dragOffset = { 0, 0 }; //offset (normalised from item center to click point

	std::vector<std::vector<DrawItem>> m_undoStack; //undo stack
	static constexpr std::size_t kMaxUndoSteps = 20; //max undo steps

	//helper to handle picking/dragging inside the viewport
	void handleViewportPicking(const ImVec2& imgMin,const ImVec2& imgMax,int vpW, int vpH);

	//undo helpers
	void pushUndoState(const char* reason = nullptr);
	void undoLastAction();
};


namespace Gui {
	// Non Member functions
	void updateImGuiManager(double& deltaTime, ImGuiManager& imGuiManager);
}
#endif // ImGuiManager_h