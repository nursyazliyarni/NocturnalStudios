/*!------------------------------------------------------------------------------------------------
    File: LevelEditor.h
    Project: NocturnalStudio

	Brief:
		This file contain the main function for the level editor
		and the functions of the editor windows.
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the LevelEditor.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Window/Window.h"
#include "ResourceManager/ResourceManager.h"
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
		void Logger();
		void Profiler();
		void Layering();
		void Hierarchy();
		void Inspector();

		//for audio loading from files
		void LoadAudioFromFile();
		void LoadScriptsFromFile();
		void AudioEditor();
		void insertAudioFilePath(std::string in);
		void insertTextureFilePath(std::string on);

		//for textures loading from files
		void LoadTextureFromFile();

		Window* GetWindow() { return mWindow; }

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
		bool FullScreenMode{};
		bool Pad{};

	private:
		
		Window* mWindow{nullptr};
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
	};
}