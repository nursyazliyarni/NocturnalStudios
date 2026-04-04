/*!------------------------------------------------------------------------------------------------
    File: LevelEditor.h
    Game Project: NocturnalStudio

	Brief:
		This file contain the main function for the level editor
		and the functions of the editor windows.
		
    Primary Author:
         LAU XIANG YANG (84%)
		- Created the LevelEditor class and its function specification.
    Secondary Author:
         Nur Syazliyarni (8%)
        - Added gizmo visibility toggle in the Scene viewer.
        - Updated Text component UI usage in the inspector.
        - Added RevealTargetOnClick script to the editor script list.
        Ang Yu Xuan (8%)
        - Adding in AnimalAttributes Component and ItemAttributes Component UI 
          onto the inspector of the level editor.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Window/Window.h"
#include "ResourceManager/ResourceManager.h"
#include "ECS/ComponentRegistry.h"
#include "core/IGameConfig.h"
#include "ExportCache.h"

#define p_Editor NocturnalEngine::LevelEditor::GetInstance()

namespace NocturnalEngine {
	class LevelEditor
	{
	public:
		//ctor and dtor
		LevelEditor() = default;
		~LevelEditor() = default;

		//main function for running
		void Init(Window* window);
		void LoadSceneFromFile(); //Load all the json file into a container for easy access in imgui
		void Update();	//update loop
		void Draw();
		void End();

		void LoadSaveScene();
		void SceneViewer();
		void ContentBrowser();
		void Logger() const;
		void Profiler();
		void Layering() const;
		void Hierarchy();
		void Inspector();
		void HandleUndoRedoShortcuts(); // Handles Ctrl+Z / Ctrl+Y / Ctrl+Shift+Z every frame.

		//for audio loading from files
		void LoadAudioFromFile();
		void LoadScriptsFromFile();
		void AudioEditor();
		void AnimationEditor();
		void insertAudioFilePath(std::string in);
		void insertTextureFilePath(std::string on);

		//for textures loading from files
		void LoadTextureFromFile();

		Window* GetWindow() { return mWindow; }

		bool ShouldEnd() const { return gameShouldClose; }

		// To get editor visibility
		static bool IsEditorActive();

	
	public:

		bool is_ShowWindow = true;
		bool imgui_mouse_control = true;
		bool scene_autobackup = false;
		bool isTransformOpen = false;

		static std::unique_ptr<LevelEditor>& GetInstance();


		//inspector and Hierarchy
		Entity selectedEntity = {};


		//for inspector and Hierarchy
		bool mDebugDraw{ false }; //to show debug/collision draw 

		//For resize and sceneviewer/viewport
		ImVec2 mViewportSize = { 0.0f, 0.0f };
		ImVec2 mViewportBounds[2];
		bool mViewportFocused = false;
		ImVec2 mGameMousePosition = { 0.0f, 0.0f };
		ImVec2 mSceneMouse = { 0.0f, 0.0f };
		

	private:
		void MainMenuBar();
		void Docking();
		void PushUndoSnapshot(); // Saves current scene state as an undo snapshot.
		void PushUndoSnapshotAndInvalidateRedo(); // Saves undo snapshot and clears redo history for new edits.
		void ApplyUndo(); // Restores previous state from undo history.
		void ApplyRedo(); // Restores next state from redo history.
		bool LoadSnapshot(const std::string& snapshotPath); // Loads a snapshot file into the current scene.
		std::string CreateSnapshotFilePath() const; // Builds a unique snapshot path in the undo folder.
		bool FullScreenMode{};
		bool Pad{};

	private:
		
		Window* mWindow{nullptr};
		//if this exists (fetches at init) this will contain all the components defined by the game side
		ComponentRegistry* mgameComponents{};
		IGameConfig* mGameConfig;
		ImGuiDockNodeFlags mDockSpaceFlags{};
		static std::unique_ptr<LevelEditor> mInstance;
		

		//profiler
		bool is_Profile = false;
		float mSceneRuntime = 0.0f;
		float mUpdateTimer = 0.0f;
		std::array<float, 5> mSystemRunTime = { 0.0f,0.0f,0.0f,0.0f };

		// memory manager
		std::size_t mMemoryBlock;
		std::size_t mMemoryPage;
		
		//Content Browser
		std::filesystem::path mAssetsPath= "Assets";
		std::filesystem::path mCurrentDirectory = mAssetsPath;
		std::filesystem::path mSelectedAsset;
		std::filesystem::path mHoveredAsset;
		
		//Scene/guizmo
		int mGizmoType = -1;
		bool mShowGizmos = true; // Toggle to hide/show ImGuizmo visuals


		//For loading of scene usage
		std::vector<std::string> mScenefile; //json file
		char mSceneInputBuffer[128] = "";
		// To keep track of the files
		int mSelectedFile = -1;
		std::vector<std::filesystem::directory_entry> mFileList;
		// For save confirmation
		std::string mLastSavedScene{};
		double mLastSaveTime = -1.0;  // store ImGui::GetTime() when saved
		double mSaveMessageDuration = 2.0; // seconds

		//To track scene editor is on (for saving purposes)
		std::string mCurrentScene{};

		//load audio from file
		std::vector<std::string> mAudioFile;
		std::vector<std::filesystem::directory_entry> mAudioFileList;
		//for displaying of audio channels
		
		//load texture from file
		std::vector<std::string> mTextureFile;
		std::vector<std::filesystem::directory_entry> mTextureFileList;

		//load script list from file
		std::vector<std::string> mScriptList;

		std::vector<std::string> mUndoStack; // Stack of snapshots for undo operations.
		std::vector<std::string> mRedoStack; // Stack of snapshots for redo operations.
		std::filesystem::path mUndoDirectory{ "Assets/Scene/.editor_undo" }; // Temporary folder for editor snapshots.
		std::size_t mMaxUndoSteps{ 30 }; // Maximum number of history entries kept.
		bool mStartedGizmoDrag{ false }; // Prevents duplicate snapshots during one gizmo drag.
		bool mInspectorEditDirty{ false }; // Tracks inspector edits until the active widget is released.

		bool mshowNewProjectPopup{ false }, //to show the user the prompt for making a new project
			mshowOpenProjectPopup{ false }, //unused, for loading up project
			mfileBrowsing{ false }, // if the user is browsing for where to place the new file
			showFoldernameInput{ false }, //do i still need this actualy idk
			setNewAsStartup{ true }, //if we want to set the new project as our startup file
			projNameUpdate{ false }, //to prevent polling of the projectmaker, this will be set to true whenever the user changes the name they wanna set their project to.
			notInsideAnotherProjFolder{ true }, //so that the user doesnt accidentally create a game inside another game
			needCloseToRload{ false }, //to show the popup prompting the user to reload
			gameShouldClose{ false }; //sets should end in enginecore
		std::filesystem::path mBrowsingPath{}, currPath{}, mSelectedProjPath{};
		std::string newFolderName{}, fixedProjName{};
		char folderNameBuffer[128] = "",currPathDisplayBuffer[128]="",selectedPathDisplayBuffer[256]="";

		char prefabNameBuffer[128] = "";
		
		bool mbuildSizeAnalyser{ false };

		/*!*************************************************************************
		This draws the modal which prompts the user to select a path and set a name
		for their new project
		****************************************************************************/
		void DrawNewProjectPopup();

		/*!*************************************************************************
		This draws the modal which prompts the user to open a Project. Since the
		editor is not able to do that yet, this is not implemented
		****************************************************************************/
		void DrawOpenProjectPopup();

		/*!*************************************************************************
		This is the file browser used for selecting a directory to load the project 
		into. I might chantge it so that if i ever implement the open project 
		function i can use it for both
		****************************************************************************/
		void StupidFileBrowser( std::filesystem::path& browsingPath, std::filesystem::path& pathReference, bool isNewProj=true);

		/*!*************************************************************************
		This draws the directory tree. Its recursive but only if the directory is open
		****************************************************************************/
		void DrawDirectoryTree(const std::filesystem::path& path, std::filesystem::path& returnPath);

		/*!*************************************************************************
		This is a little popup function for the file browser if u right click the
		folder or the explorer u can make a new folder
		****************************************************************************/
		void NewFolderPopupThingy(std::filesystem::path newFolderParent );

		/*!*************************************************************************
		This just notifies the user that they need to close the engine in order to
		load the project
		****************************************************************************/
		void ShowReloadPrompt();

		void DrawHierarchyNode(Entity e);

		void DrawBuildSizeAnalyser();

		ExportCache mbuildCache{};
		bool refreshBuildsize{ true };
		unsigned long long totalBuildBytes{ 0 };
		std::filesystem::path exportPath{}, exportParentPath{};
		void AssetListSelectableTree(const std::filesystem::path& path,bool parentSelectionChanged=false,bool newSelection=false);
		void RefreshBuildSize();
		void ExportSelected(const std::filesystem::path& dest);
		void LoadExportCache();
		std::string FormatSize(std::uintmax_t size);
	};
}
