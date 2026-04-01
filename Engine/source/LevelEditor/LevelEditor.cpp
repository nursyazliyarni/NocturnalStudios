/*!------------------------------------------------------------------------------------------------
    File: LevelEditor.cpp
    Game Project: NocturnalStudio

	Brief:
        The level editor utilises Dear ImGui and OpenGL to create a editor interface
        to allow us to edit object, create entities and modify the
        properties of the components and save them accordingly.
	
    Primary Author:
         LAU XIANG YANG (70%)
		- Created the LevelEditor class and its function specification.
    Secondary Author:
         Nur Syazliyarni (5%)
        - Added gizmo visibility toggle in the Scene viewer.
        - Updated Text component UI usage in the inspector.
        - Added RevealTargetOnClick script to the editor script list.
        Ang Yu Xuan (5%)
        - Adding in AnimalAttributes Component and ItemAttributes Component UI 
          onto the inspector of the level editor.
        LAYCOCK ENHUI SARAH (20%)
        - Added functionality for content browser, allowing for tetxures to be 
          selected and viewed directly in content browser instead of opening up 
          a seperate tab in imgui. 
        - Added scene saving functionality
        - Added placing textures directly into game scenes as entities via dragging
          & dropping from the content browser
        - loading of scenes directly from the content browser
        (22/02/2026)
        - Added the capability to make a new game project. This includes a popup modal, a file
          browser, and a prompt for the user to close the window afterwards.
        - Made the inspector loop much cleaner - the leveleditor now calls the component
          registry and iterates through it to display. When adding a new component, you 
          no longer need to edit the leveleditor file. You must make sure to populate
          the ToEditor() function in yout component. Check each component's cpp file to
          see how its done.
        - Changed the logic for deleting entities so that it no longer behaves weirdly when 
          deleting the top item in the list.


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include "GUI/GUI.h"

#include "LevelEditor/LevelEditor.h"
#include "Timer/Time.h"
#include "Timer/Fps.h"
#include "Graphics/Renderer.h"
#include "Graphics/Graphics.h"
#include "LevelEditor/Picker.h"
#include "ECS/ParentingSystem.h"

#include "Graphics/FrameBuffer.h"
#include "Core/Log.h"
#include "Audio/AudioEngine.h"
#include "Animation/Animation.h"

#include "ECS/Components/Components.h"
#include "ECS/SceneManager.h"
#include "ECS/EngineComponentReg.h"
#include "Core/MemoryManager.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "System/ServiceLocator.h"

#include "ResourceManager/NewProjectMaker.h"

#include <unordered_map>
#include <array>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdint>




namespace NocturnalEngine {

    static std::unordered_map<std::string,
        std::array<bool, NocturnalEngine::Graphic::MAX_LAYERS>> gSceneLayerState;

    std::unique_ptr<LevelEditor> LevelEditor::mInstance{ nullptr };

    std::unique_ptr<LevelEditor>& LevelEditor::GetInstance()
    {
        if (mInstance == nullptr)
        {
            mInstance = std::make_unique<LevelEditor>();
        }
        return mInstance;
     }
    
    bool LevelEditor::IsEditorActive()
    {
		return mInstance != nullptr && mInstance->is_ShowWindow;
    }

    bool color_picker = false;
    bool drop_menu = false;
    bool logger = false;
    bool HaltUpdates = true;

    static int current_sound = 0;
    /*!*************************************************************************
    Init loop for levelEditor sets context for ImGui
    ****************************************************************************/
    void LevelEditor::Init(Window* window)
    {

        if (!window || !window->GetWindow()) return;
        mWindow = window;

        if (!std::filesystem::exists(mAssetsPath))
        {
            NOCTURNAL_ENGINE_WARN("Assets/ folder missing! Creating it...");
            std::filesystem::create_directories(mAssetsPath);
            
        }

       
        // Ensure the snapshot folder exists and starts clean each run.
        std::filesystem::create_directories(mUndoDirectory);
        for (const auto& entry : std::filesystem::directory_iterator(mUndoDirectory))
        {
            if (entry.is_regular_file())
                std::filesystem::remove(entry.path());
        }
        // Reset in-memory history state.
        mUndoStack.clear();
        mRedoStack.clear();
        mStartedGizmoDrag = false;
        mInspectorEditDirty = false;
        
        IMGUI_CHECKVERSION();

        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();

        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

        ImGui_ImplGlfw_InitForOpenGL(window->GetWindow(), true);

        ImGui_ImplOpenGL3_Init("#version 450");

        LoadAudioFromFile();
        std::filesystem::create_directories("Assets/Video");

        LoadSceneFromFile();

        mGameConfig= ServiceLocator::Get().GetGameConfig();
        mgameComponents = ServiceLocator::Get().GetGameComponents();
        LoadScriptsFromFile();
        LoadExportCache();
        // Seed undo history with the initial editor state.
        PushUndoSnapshot();

    }
    /*!*************************************************************************
    Update loop for level editor, poll events and set new frames
    ****************************************************************************/
    void LevelEditor::Update()
    {      
#if DEBUG
        if (p_Input->KeyPressed(GLFW_KEY_F1))
        {
            if (is_ShowWindow) {
                is_ShowWindow = !is_ShowWindow;
            }
        }

        ImGuiIO& io = ImGui::GetIO();
        //press F2 to switch mouse control from imgui to game
        if (p_Input->KeyPressed(GLFW_KEY_F2)&&imgui_mouse_control)
        {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse; //the game own the mouse control
            imgui_mouse_control = false;
            ImGuizmo::Enable(false);
        }
        //press F2 to switch mouse control from game to imgui
        else if (p_Input->KeyPressed(GLFW_KEY_F2) && !imgui_mouse_control) {
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse; //imgui own the mouse control
            imgui_mouse_control = true;
            ImGuizmo::Enable(true);
        }
#endif
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // Check keyboard shortcuts before drawing UI.
        HandleUndoRedoShortcuts();
        MainMenuBar();

        // Draw modals outside BeginMainMenuBar
        if (mshowNewProjectPopup) DrawNewProjectPopup();
        if (mshowOpenProjectPopup) DrawOpenProjectPopup();
        if (mbuildSizeAnalyser) DrawBuildSizeAnalyser();
        if (needCloseToRload)ShowReloadPrompt();

        if (is_ShowWindow)
        { 
            Docking();
            /*MainMenuBar();*/
            LoadSaveScene();
            Profiler();
            Layering();
            ContentBrowser();
            Logger();
            Hierarchy();
            Inspector();
            SceneViewer();
            AudioEditor();
            AnimationEditor();
        }

    }

    /*!*************************************************************************
   Render interface onto frame
   ****************************************************************************/

    void LevelEditor::Draw()
    {
       if (!ImGui::GetCurrentContext())
           return;

        ImGui::Render();

        ImDrawData* draw_data = ImGui::GetDrawData();
        if (draw_data) 
            ImGui_ImplOpenGL3_RenderDrawData(draw_data);
        
        // const ImGuiIO& io = ImGui::GetIO();
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            auto* Window = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(Window);
        }

    }

    /*!*************************************************************************
    End instance of the level editor
    ****************************************************************************/
    void LevelEditor::End()
    {
        // Remove tracked snapshot files.
        for (const auto& path : mUndoStack)
            std::filesystem::remove(path);
        for (const auto& path : mRedoStack)
            std::filesystem::remove(path);
        // Remove any leftover files in undo folder.
        for (const auto& entry : std::filesystem::directory_iterator(mUndoDirectory))
        {
            if (entry.is_regular_file())
                std::filesystem::remove(entry.path());
        }
        mUndoStack.clear();
        mRedoStack.clear();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    /*!*************************************************************************
    Load file paths for scene from assets folder
    ****************************************************************************/
    void LevelEditor::LoadSceneFromFile()
    {
        std::string path = "Assets/Scene";
        for (auto const& dir_entry : std::filesystem::directory_iterator{ path })
        {
            //Checks if the given file status or path corresponds to a regular file
            if (!dir_entry.is_regular_file())
            {
                continue;
            }

            mFileList.emplace_back(dir_entry);
            // used to Load Scene
            mScenefile.emplace_back(dir_entry.path().filename().string());
        }
    }

    /*!*************************************************************************
    Load file paths for textures from assets folder
    ****************************************************************************/
    void LevelEditor::LoadTextureFromFile()
    {
        std::string path = "Assets/Textures";

        if (!std::filesystem::exists(path))
        {
            NOCTURNAL_ENGINE_WARN(path+" folder missing! Creating it...");
            std::filesystem::create_directories(path);
            if (!std::filesystem::exists(path)) {
                NOCTURNAL_ENGINE_WARN("Assets/Audio/ Failed to create! Audio Init aborted...");
                return;
            }
        }

        for (auto const& dir_entry : std::filesystem::directory_iterator{ path })
        {
            //Checks if the given file status or path corresponds to a regular file
            if (!dir_entry.is_regular_file())
            {
                continue;
            }

            mTextureFileList.emplace_back(dir_entry);
            // used to Load Scene
            mTextureFile.emplace_back(dir_entry.path().filename().string());
        }
    }

   /*!*************************************************************************
   Loads filepaths for audio files from assets folder
   ****************************************************************************/
    void LevelEditor::LoadAudioFromFile()
    {
        std::string audio_path = "Assets/Audio";

        if (!std::filesystem::exists(audio_path))
        {
            NOCTURNAL_ENGINE_WARN("Assets/Audio/ folder missing! Creating it...");
            std::filesystem::create_directories(audio_path);
            if (!std::filesystem::exists(audio_path)) {
                NOCTURNAL_ENGINE_WARN("Assets/Audio/ Failed to create! Audio Init aborted...");
                return;
            }
        }

        for (auto const& dir_entry : std::filesystem::directory_iterator{ audio_path }) //iterate files in Audio folder
        {
            //checks if given file status or path corresponds to a regular file
            if (!dir_entry.is_regular_file())
            {
                continue;
            }

            mAudioFileList.emplace_back(dir_entry);
            // used to load audio
            mAudioFile.emplace_back(dir_entry.path().filename().string());

        }
    }

    /*!*************************************************************************
    Script filepath to get list of scripts
    ****************************************************************************/
    void LevelEditor::LoadScriptsFromFile()
    {

    }

    /*!*************************************************************************
    Menu bar located in the top left side of the window is used to toggle between
    opening and closing the editor
    ****************************************************************************/
    void LevelEditor::MainMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Game Project"))
                {
                    if (is_ShowWindow) { mshowNewProjectPopup = true; 
                    mSelectedProjPath = std::filesystem::current_path().parent_path();
                    mBrowsingPath = std::filesystem::current_path().parent_path();
                    fixedProjName = "";
                    notInsideAnotherProjFolder = true;
                    }
                }
                if (ImGui::MenuItem("Build Size Analyser"))
                {
                    if (is_ShowWindow) mbuildSizeAnalyser = true;
                }
                ImGui::EndMenu();
            }
            {
                const bool canUndo = mUndoStack.size() > 1;
                const bool canRedo = !mRedoStack.empty();
                ImGui::BeginDisabled(!canUndo);
                if (ImGui::Button("Undo"))
                    ApplyUndo();
                ImGui::EndDisabled();

                ImGui::SameLine();

                ImGui::BeginDisabled(!canRedo);
                if (ImGui::Button("Redo"))
                    ApplyRedo();
                ImGui::EndDisabled();

                ImGui::SameLine();
                ImGui::TextUnformatted("Ctrl+Z / Ctrl+Y");
            }

            const char* playLabel = is_ShowWindow ? "Play" : "Stop";
            const float buttonWidth = ImGui::CalcTextSize(playLabel).x + ImGui::GetStyle().FramePadding.x * 2.0f;
            const float centeredX = (ImGui::GetWindowSize().x - buttonWidth) * 0.5f;
            if (centeredX > ImGui::GetCursorPosX())
                ImGui::SetCursorPosX(centeredX);

            if (is_ShowWindow)
            {
                if (ImGui::Button("Play"))
                {
                    is_ShowWindow = false;

                    
                    if (mGameConfig) {  // Null check still works
                        if (mGameConfig->IsPlayableScene(mCurrentScene)) {
                            p_Scene->isGameplay = true;
                            
                        }
                        mGameConfig->OnEnterPlayMode(mCurrentScene);
                    }
                }
               
            }
            else
            {
                if (ImGui::Button("Stop"))
                {
                    is_ShowWindow = true;
                    p_Scene->isGameplay = false;
                    mGameConfig->OnExitPlayMode(true);
                    if (!mGameConfig->mainMenuScene.empty()) { 
                        p_Scene->setSceneToLoad(mGameConfig->mainMenuScene); 
                        mCurrentScene = mGameConfig->mainMenuScene;
                    }
                    p_Audio->StopChannel(CAudioEngine::channel_groups::master);
                    
                    
                    
                }
            }
        }
		ImGui::EndMainMenuBar();
    }

    /*!*************************************************************************
   Load and save scenes by serialising and deserializing data from JSON files
   ****************************************************************************/
    void LevelEditor::LoadSaveScene()
    {
        std::string path = "Assets/Scene/";
        ImGui::Begin("Save Scene");
        // --- Input text for filename ---
        ImGui::InputText("Scene Name", mSceneInputBuffer, sizeof(mSceneInputBuffer));

        ImGui::Spacing();
        // --- Save button ---
        if (ImGui::Button("SAVE"))
        {
            std::string filename = mSceneInputBuffer;

            if (filename.empty())
            {
                std::cout << "Cannot save: filename is empty.\n";
            }
            else if(mLastSaveTime < 0.f)
            {
                // Ensure extension
                if (filename.find(".json") == std::string::npos)
                    filename += ".json";


                std::string fullPath = path + filename;

                // Save scene
                
                if (p_Scene->SerializeToFile(fullPath))
                {
                    mLastSavedScene = filename;
                    mCurrentScene = filename;
                    mLastSaveTime = ImGui::GetTime();
                }

                //std::cout << "Saved scene: " << fullPath << "\n";

                
            }
        }
        // Same line as Save button
        ImGui::SameLine();
        if (ImGui::Button("QUICK SAVE"))
        {
            std::string filename = mCurrentScene;

            if (filename.empty())
            {
                //std::cout << "Cannot save: filename is empty.\n";
                filename = "Scene.json";
            }
            if (mLastSaveTime < 0.f)
            {
                // Ensure extension
                if (filename.find(".json") == std::string::npos)
                    filename += ".json";


                std::string fullPath = path + filename;

                // Save scene

                if (p_Scene->SerializeToFile(fullPath))
                {
                    mLastSavedScene = filename;
                    mCurrentScene = filename;
                    mLastSaveTime = ImGui::GetTime();
                }

                //std::cout << "Saved scene: " << fullPath << "\n";


            }
        }
        ImGui::SameLine();
        if (ImGui::Button("NEW SCENE"))
        {

            if (mLastSaveTime < 0.f)
            {
                p_Scene->setSceneToLoad(" ");
                mCurrentScene = "Assets/Scene/NewScene.json";
                mLastSaveTime = ImGui::GetTime();


            }
        }

        // Show message if recently saved
        if (mLastSaveTime > 0.0f && ImGui::GetTime() - mLastSaveTime < mSaveMessageDuration)
        {
            std::string saveMessage = mLastSavedScene + " has been saved in "+path+"!";
            ImGui::TextColored(ImVec4(1.f,1.f,1.f,  static_cast<float>(((mLastSaveTime+mSaveMessageDuration)- ImGui::GetTime())/mSaveMessageDuration)), saveMessage.c_str());
            
        }
        if (mLastSaveTime > 0.0f && ImGui::GetTime() - mLastSaveTime > mSaveMessageDuration)mLastSaveTime = -1.f;

        
        
        
        ImGui::End();
    }

    /*!*************************************************************************
    Scene viewer
    ****************************************************************************/
    void LevelEditor::SceneViewer()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Scene");
        ImGui::Checkbox("Show Gizmos", &mShowGizmos); // User toggle for gizmo visibility

        auto vpMinRegion = ImGui::GetWindowContentRegionMin();
        auto vpMaxRegion = ImGui::GetWindowContentRegionMax();
        auto vpOffset = ImGui::GetWindowPos();

        mViewportBounds[0] = { vpMinRegion.x + vpOffset.x, vpMinRegion.y + vpOffset.y };
        mViewportBounds[1] = { vpMaxRegion.x + vpOffset.x, vpMaxRegion.y + vpOffset.y };

        //for scrolling purpose
        mViewportFocused = ImGui::IsWindowFocused();
        mViewportSize = { ImGui::GetContentRegionAvail() };

        uint64_t textureID = p_FrameBuffer->GetColorAttachmentRendererID();
        ImGui::Image((ImTextureID)textureID, mViewportSize,
            ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Textures"))
            {
                Entity newEntity = p_ecs.CreateEntity();
                Transform trans;
                trans.SetPos(0.f,0.f);
                trans.SetScale(1.0f, 1.0f);
                trans.SetRot(0.0f);
                p_ecs.AddComponent<Transform>(newEntity, trans);
                Sprite sprite{};
                sprite.SetTexture(*(const std::string*)payload->Data);
                p_ecs.AddComponent<Sprite>(newEntity, sprite);
                selectedEntity = newEntity;
                std::string eName = *(const std::string*)payload->Data; 
                if (eName.find_last_of("/")!=std::string::npos)
                {
                    eName = eName.substr(eName.find_last_of("/") + 1);
                }
                NameTag nt{};
                nt.SetNameTag(eName);                                   
                p_ecs.AddComponent<NameTag>(newEntity, nt);
                PushUndoSnapshotAndInvalidateRedo();
            }
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Videos"))
            {
                const std::string videoPath = static_cast<const char*>(payload->Data);
                Entity newEntity = p_ecs.CreateEntity();

                Transform trans;
                trans.SetPos(0.f, 0.f);
                trans.SetScale(1.777f, 1.0f);
                trans.SetRot(0.0f);
                p_ecs.AddComponent<Transform>(newEntity, trans);

                Sprite sprite{};
                sprite.SetTexture("Blank");
                p_ecs.AddComponent<Sprite>(newEntity, sprite);

                Video video{};
                video.SetVideoPath(videoPath);
                video.playInEditor = false;
                p_ecs.AddComponent<Video>(newEntity, video);

                NameTag nt{};
                std::string entityName = std::filesystem::path(videoPath).stem().string();
                nt.SetNameTag(entityName);
                p_ecs.AddComponent<NameTag>(newEntity, nt);

                selectedEntity = newEntity;
                PushUndoSnapshotAndInvalidateRedo();
            }
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PrefabDrop"))
            {
                std::string prefabName = static_cast<const char*>(payload->Data);

                // Load prefab if not already loaded
                p_PrefabManager.CheckOrAddPrefabFromName(prefabName);

                if (p_PrefabManager.DoesPrefabExist(prefabName))
                {
                    Entity newEntity = p_PrefabManager.CloneFromPrefab(prefabName);
                    if (newEntity != MAX_ENTITIES)
                    {
                        selectedEntity = newEntity;
                        PushUndoSnapshotAndInvalidateRedo();
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        //for mouse position
        mGameMousePosition = ImGui::GetMousePos();

        mGameMousePosition.x -= mViewportBounds[0].x;
        mGameMousePosition.y -= mViewportBounds[0].y;

        glm::vec2 vpSize{ 0.0f ,0.0f };
        vpSize.x = mViewportBounds[1].x -mViewportBounds[0].x;
        vpSize.y = mViewportBounds[1].y - mViewportBounds[0].y;
        mGameMousePosition.y = vpSize.y - mGameMousePosition.y;
        mGameMousePosition.x = ((mGameMousePosition.x / mViewportSize.x) * 2.0f) - 1.0f;
        mGameMousePosition.y = ((mGameMousePosition.y / mViewportSize.y) * 2.0f) - 1.0f;
        static bool dragging = false;
        static double c_x = 0.0f, c_y = 0.0f;
        if (p_Input->MousePressed(GLFW_MOUSE_BUTTON_RIGHT))
        {
            if (mSceneMouse.x >= mViewportBounds[0].x &&
                mSceneMouse.x <= mViewportBounds[1].x &&
                mSceneMouse.y >= mViewportBounds[0].y &&
                mSceneMouse.y <= mViewportBounds[1].y)
            {
                dragging = true;
            }
        }
        if (dragging && p_Input->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
        {
            double new_x, new_y;
            glfwGetCursorPos(mWindow->GetWindow(), &new_x, &new_y);
            
            NocturnalEngine::Graphic::scene_camera.SetPosition({ NocturnalEngine::Graphic::scene_camera.GetPosition().x - (static_cast<float>(new_x - c_x) * 0.005f) , 
                                                    NocturnalEngine::Graphic::scene_camera.GetPosition().y + (static_cast<float>(new_y - c_y) * 0.005f),
                0.0f });
           
        }
        else if (dragging && p_Input->MouseIsReleased(GLFW_MOUSE_BUTTON_RIGHT))
        {
            dragging = false;
        }
        glfwGetCursorPos(mWindow->GetWindow(), &c_x, &c_y);
        //gizmos
        if (p_Input->isKeyPressed(GLFW_KEY_1) && !ImGuizmo::IsUsing() && mViewportFocused)
        {
            mGizmoType = ImGuizmo::OPERATION::TRANSLATE;
        }
        else if (p_Input->isKeyPressed(GLFW_KEY_2) && !ImGuizmo::IsUsing() && mViewportFocused)
        {
            mGizmoType = ImGuizmo::OPERATION::ROTATE;
        }
        else if (p_Input->isKeyPressed(GLFW_KEY_3) && !ImGuizmo::IsUsing() && mViewportFocused)
        {
            mGizmoType = ImGuizmo::OPERATION::SCALE;
        }
        if (mShowGizmos) // Skip ImGuizmo draw path when hidden
        {
            ImGuizmo::BeginFrame();
            if (selectedEntity >= 0 && selectedEntity < MAX_ENTITIES && p_ecs.HaveComponent<Transform>(selectedEntity))//have selected entity
            {
                ImGuizmo::SetOrthographic(true);


                ImGuizmo::SetRect(
                    mViewportBounds[0].x,
                    mViewportBounds[0].y,
                    mViewportBounds[1].x - mViewportBounds[0].x,
                    mViewportBounds[1].y - mViewportBounds[0].y
                );

                glm::mat4 cameraProj = NocturnalEngine::Graphic::mcamera->GetProjectionMatrix();
                glm::mat4 cameraView = NocturnalEngine::Graphic::mcamera->GetViewMatrix();
                glm::mat4 transform{ 1.0f }; // identity matrix

                auto& trans = p_ecs.GetComponent<Transform>(selectedEntity);

                transform = glm::translate(glm::mat4{ 1.0f }, glm::vec3(trans.GetPos().x, trans.GetPos().y, 0.0f))
                    * glm::rotate(glm::mat4(1.0f), glm::radians(trans.GetRot()), glm::vec3(0.0f, 0.0f, 1.0f))
                    * glm::scale(glm::mat4(1.f), { trans.GetScale().x,trans.GetScale().y ,1.0f });

                ImGuizmo::SetDrawlist();
                // Draw ImGuizmo (renders every frame)
                ImGuizmo::Manipulate(glm::value_ptr(cameraView)
                    , glm::value_ptr(cameraProj), (ImGuizmo::OPERATION)mGizmoType,
                    ImGuizmo::WORLD, glm::value_ptr(transform));

                if (ImGuizmo::IsUsing())
                {
                    mStartedGizmoDrag = true;
                    glm::vec3 decompTrans, decompRot, decompScale;

                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform),
                        glm::value_ptr(decompTrans),
                        glm::value_ptr(decompRot),
                        glm::value_ptr(decompScale));
                    // change size 
                    trans.SetPos(decompTrans[0], decompTrans[1]);
                    trans.SetRot(decompRot[2]);
                    trans.SetScale(decompScale[0], decompScale[1]);

                }
                else if (mStartedGizmoDrag)
                {
                    PushUndoSnapshotAndInvalidateRedo();
                    mStartedGizmoDrag = false;
                }
            }
        }
        else
        {
            mStartedGizmoDrag = false;
        }

        ImGui::End();
        ImGui::PopStyleVar();
        std::multimap<float, Transform*> sortedMultimap;
        for (Entity entity = 0; entity < p_ecs.GetTotalSceneEntities(); entity++)
        {
            if(p_ecs.HaveComponent<Transform>(entity))
                sortedMultimap.insert({ p_ecs.GetComponent<Transform>(entity).GetRot(),
                            &p_ecs.GetComponent<Transform>(entity) });
        }
        if (p_ecs.GetTotalSceneEntities() != 0 && p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT) && (!mShowGizmos || !ImGuizmo::IsOver())) // Allow picking when gizmos are hidden
        {
            Entity picked = (Entity)Picker::Pick(NocturnalEngine::Graphic::mcamera, sortedMultimap);

            if (picked >= 0 && picked < MAX_ENTITIES &&
                p_ecs.GetAliveEntitiesSet().contains(picked) &&  // ← key check
                p_ecs.HaveComponent<Transform>(picked))
            {
                selectedEntity = picked;
            }
            else
            {
                selectedEntity = MAX_ENTITIES;
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
        {
            if (selectedEntity >= 0 && selectedEntity < MAX_ENTITIES)
            {
                p_Heirarchy.DestroyHierarchy(selectedEntity);
                selectedEntity = MAX_ENTITIES;
                PushUndoSnapshotAndInvalidateRedo();
            }
        }
    }

    /*!*************************************************************************
   View various assets from the assets folders from the level editor
   ****************************************************************************/
    void LevelEditor::ContentBrowser()
    {
        if (is_ShowWindow)
        {
            ImGui::Begin("Content Browser");


            //START back button & and breadcrumbsection
            
            if (mCurrentDirectory != std::filesystem::path(mAssetsPath))
            {
                ImGui::BeginGroup();
                if(ImGui::ImageButton("BackButton", (ImTextureID)(intptr_t)ResourceManager::GetIcon("BackIcon")->GetRendererID(),
                    ImVec2(25.f, 25.f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)))
                {
                    mCurrentDirectory = mCurrentDirectory.parent_path();
                }
                ImGui::SameLine();
                ImGui::Dummy(ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));
                ImGui::SameLine();

                std::filesystem::path curPath = mCurrentDirectory;
                std::vector<std::filesystem::path> segments;
                
                // Build segments
                while (curPath != curPath.root_path())
                {
                    segments.push_back(curPath.filename());
                    curPath = curPath.parent_path();
                }
                segments.push_back(curPath); // root
                std::reverse(segments.begin(), segments.end());

                // Render breadcrumbs
                std::filesystem::path accumulated;

                for (int i{ 1 }; i < segments.size(); ++i)
                {
          
                    if (i > 1)
                    {
                        ImGui::SameLine();
                        ImGui::Text("/");
                        ImGui::SameLine();
                    }

                    accumulated /= segments[i];

                    ImGui::PushID(i);
                    if (ImGui::SmallButton(segments[i].string().c_str()))
                    {
                        mCurrentDirectory = accumulated;
                    }
                    ImGui::PopID();

                    ImGui::SameLine();
                }
                ImGui::EndGroup();
                ImGui::Separator();
            }
            
            //END back button & breadcrumb section


            float padding{ 16.0f }, thumbnailSize{ 128.0f }, cellBaseSize{ thumbnailSize + padding }, panelWidth{ ImGui::GetContentRegionAvail().x };
            int columnCount = panelWidth / cellBaseSize < 1.f ? 1 : (int)(panelWidth / cellBaseSize), colPos{ 0 };


            for (auto& Directory : std::filesystem::directory_iterator(mCurrentDirectory))
            {
                const auto& directorypath = Directory.path(); //the path for folders in Assets(assets/fonts)

                std::string filename = directorypath.filename().string(); //just the name of the folders(fonts, icons, Audio etc)
                ImGui::PushID(filename.c_str());
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

                ImGui::BeginGroup();
                //check what type the directory is
                if (Directory.is_directory())
                {
                    ImGui::ImageButton("FolderIcon", (ImTextureID)(intptr_t)ResourceManager::GetIcon("FolderIcon")->GetRendererID(),
                        ImVec2(thumbnailSize,thumbnailSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                }

                //dealing w diff filenames
                auto ext = directorypath.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); // lowercase for safety

                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
                {
                    std::string gs = directorypath.generic_string();

                    // Use relative path as key to avoid name collisions
                    // e.g. "Characters/Cat" instead of just "Cat"
                    std::string relativePath = std::filesystem::relative(
                        directorypath,
                        std::filesystem::path("Assets/Textures/")
                    ).generic_string();

                    // Remove extension from key
                    std::string texName = relativePath.substr(0, relativePath.find_last_of('.'));

                    auto tex = ResourceManager::GetTexture(texName);
                    if (!tex) tex = ResourceManager::GetIcon(texName);

                    if (tex)
                    {
                        
                        // Scale to fit thumbnail
                        float w{ static_cast<float>(tex->GetWidth()) }, h{ static_cast<float>(tex->GetHeight()) };
                        float scale = thumbnailSize / std::max(w, h);
                        ImVec2 imgSize(w * scale, h * scale);

                        // --- Fixed-size thumbnail box with bottom-left alignment ---
                        ImGui::BeginGroup(); // box container
                        ImGui::Dummy(ImVec2(thumbnailSize - imgSize.x, 0.0f)); // horizontal padding
                        ImGui::SameLine();
                        ImGui::BeginGroup();
                        ImGui::Dummy(ImVec2(0, thumbnailSize - imgSize.y)); // vertical padding above image
                        ImGui::Image((ImTextureID)(intptr_t)tex->GetRendererID(), imgSize,ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                        ImGui::EndGroup();
                        ImGui::EndGroup();

                        if (ImGui::IsItemHovered())
                        {
                            ImGui::BeginTooltip();
                            std::string str = texName.substr(texName.find_last_of('/')+1,texName.length()- (texName.find_last_of('/')+1)) + " " + "(" + std::to_string(tex->GetWidth()) + "x" +
                                std::to_string(tex->GetHeight()) + ")";
                            ImGui::Text(str.c_str());
                            ImGui::EndTooltip();
                        }

                        if (mCurrentDirectory.filename() != "Icons"&&ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                        {
                            auto it = ResourceManager::textures.find(texName);
                            auto& addN = it->first;
                            ImGui::SetDragDropPayload("Textures",&addN,sizeof(addN));
                            //std::cout << payloadStr;

                            ImGui::Image((ImTextureID)(intptr_t)tex->GetRendererID(),
                                ImVec2(thumbnailSize * static_cast<float>(tex->GetWidth()) / static_cast<float>(tex->GetHeight()), thumbnailSize),
                                ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                            ImGui::EndDragDropSource();
                        }
                    }
                }
                else if (ext==".json") {

                    // Pick icon based on which folder we're in
                    const char* jsonIcon = "TextIcon";
                    if (mCurrentDirectory.filename() == "Prefabs")jsonIcon = "PrefabIcon";
                    ImGui::ImageButton("SceneButton",(ImTextureID)(intptr_t)ResourceManager::GetIcon(jsonIcon)->GetRendererID(),
                        ImVec2(thumbnailSize, thumbnailSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

                    if (mCurrentDirectory.filename() == "Prefabs" &&
                        ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        std::string prefabName = filename.substr(0, filename.find_last_of('.'));
                        ImGui::SetDragDropPayload("PrefabDrop", prefabName.c_str(), prefabName.size() + 1);
                        ImGui::Image((ImTextureID)(intptr_t)ResourceManager::GetIcon(jsonIcon)->GetRendererID(),
                            ImVec2(thumbnailSize, thumbnailSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                        ImGui::Text("%s", prefabName.c_str());
                        ImGui::EndDragDropSource();
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        std::string jsnName = filename;
                        jsnName.erase(filename.find_last_of('.'));

                        std::string str{};

                        if (mCurrentDirectory.filename() == "Scene") {
                            str += "Scene: ";
                        }
                        else if (mCurrentDirectory.filename() == "Prefabs") {
                            str += "Prefab: ";
                        }
                        str += jsnName;

                        ImGui::Text(str.c_str());
                        ImGui::EndTooltip();

                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {

                            if (scene_autobackup&&!mCurrentScene.empty()&&mCurrentScene!=filename&&mLastSavedScene!=mCurrentScene)
                            {
                                std::string backupScene = mCurrentScene;
                                backupScene.erase(mCurrentScene.find_last_of('.'));
                                p_Scene->SerializeToFile(mCurrentDirectory.string() + '/' + backupScene + "_AutoBackup.json");
                                mLastSavedScene = mCurrentScene;
                            }

                            // SAVE current scene layer state BEFORE switching away
                            if (!mCurrentScene.empty())
                            {
                                auto& saved = gSceneLayerState[mCurrentScene];
                                for (int l = 0; l < NocturnalEngine::Graphic::MAX_LAYERS; ++l)
                                    saved[l] = NocturnalEngine::Graphic::layerEnabled[l];
                            }

                            p_Scene->DeserializeFromFile(mCurrentDirectory.string() + '/' + filename);
                            mCurrentScene = filename;
                            // New scene load starts a fresh history chain.
                            for (const auto& path : mUndoStack)
                                std::filesystem::remove(path);
                            for (const auto& path : mRedoStack)
                                std::filesystem::remove(path);
                            mUndoStack.clear();
                            mRedoStack.clear();
                            mInspectorEditDirty = false;
                            // Keep one baseline snapshot for the loaded scene.
                            PushUndoSnapshot();
                            selectedEntity = MAX_ENTITIES;

                            //  RESTORE layer state for newly loaded scene 
                            auto it = gSceneLayerState.find(mCurrentScene);
                            if (it != gSceneLayerState.end())
                            {
                                for (int l = 0; l < NocturnalEngine::Graphic::MAX_LAYERS; ++l)
                                    NocturnalEngine::Graphic::layerEnabled[l] = it->second[l];
                            }
                            else
                            {
                                // First time opening this scene -> default all ON
                                for (int l = 0; l < NocturnalEngine::Graphic::MAX_LAYERS; ++l)
                                    NocturnalEngine::Graphic::layerEnabled[l] = true;
                            }

                            
                        }
                    }
                    
                }
                else if (ext == ".wav"|| ext == ".ogg"|| ext == ".mp3") {
                    std::string sndName = filename;
                    sndName.erase(filename.find_last_of('.'));

                    ImGui::ImageButton("SoundButton", (ImTextureID)(intptr_t)ResourceManager::GetIcon("SoundIcon")->GetRendererID(),
                        ImVec2(thumbnailSize, thumbnailSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        

                        std::string str = sndName;

                        ImGui::Text(str.c_str());
                        ImGui::EndTooltip();

                        
                    }
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        std::string payloadStr = sndName;
                        std::string relativePath = filename;
                        if (mCurrentDirectory.filename() != "Audio")
                        {
                            relativePath = std::filesystem::relative(directorypath, std::filesystem::path("Assets/Audio")).generic_string();
                        }
                        ImGui::SetDragDropPayload("AudioFiles", relativePath.c_str(), relativePath.size() + 1);

                        ImGui::Image((ImTextureID)(intptr_t)ResourceManager::GetIcon("SoundIcon")->GetRendererID(),
                            ImVec2(thumbnailSize, thumbnailSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                        ImGui::EndDragDropSource();
                    }

                }
                else if (ext == ".mpg" || ext == ".mpeg") {
                    std::string videoName = filename;
                    videoName.erase(filename.find_last_of('.'));

                    ImGui::ImageButton("VideoButton", (ImTextureID)(intptr_t)ResourceManager::GetIcon("TextIcon")->GetRendererID(),
                        ImVec2(thumbnailSize, thumbnailSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("Video: %s", videoName.c_str());
                        ImGui::EndTooltip();
                    }

                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        std::string relativePath = filename;
                        if (mCurrentDirectory.filename() != "Video")
                        {
                            relativePath = std::filesystem::relative(directorypath, std::filesystem::path("Assets/Video")).generic_string();
                        }
                        else
                        {
                            relativePath = directorypath.filename().generic_string();
                        }

                        ImGui::SetDragDropPayload("Videos", relativePath.c_str(), relativePath.size() + 1);
                        ImGui::Image((ImTextureID)(intptr_t)ResourceManager::GetIcon("TextIcon")->GetRendererID(),
                            ImVec2(thumbnailSize, thumbnailSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                        ImGui::EndDragDropSource();
                    }
                }
                else if (ext == ".cpp" || ext == ".hpp" || ext == ".h"||ext==".c") {
                    ImGui::Image( (ImTextureID)(intptr_t)ResourceManager::GetIcon("TextIcon")->GetRendererID(),
                        ImVec2(thumbnailSize, thumbnailSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                }
                else if (ext == ".ttf"||ext==".otf") {
                    ImGui::Image((ImTextureID)(intptr_t)ResourceManager::GetIcon("FontIcon")->GetRendererID(),
                        ImVec2(thumbnailSize, thumbnailSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                }
                else if (ext == ".shader") {
                    ImGui::Image((ImTextureID)(intptr_t)ResourceManager::GetIcon("ShaderIcon")->GetRendererID(),
                        ImVec2(thumbnailSize, thumbnailSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                }
                

                //Press into the folder
                ImGui::PopStyleColor();
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (Directory.is_directory())
                        mCurrentDirectory /= directorypath.filename();
                }


                std::string displayName = filename;
                while (ImGui::CalcTextSize(displayName.c_str()).x > thumbnailSize-3 && !displayName.empty()) {
                    displayName.pop_back();
                    
                }
                if (displayName!=filename&&ImGui::CalcTextSize(displayName.c_str()).x <=thumbnailSize - 3)displayName += "...";

                ImGui::TextUnformatted(displayName.c_str());


                ImGui::EndGroup(); //indicate the folder name
                ImGui::PopID();

                ++colPos;
                if (colPos<columnCount)
                {
                    ImGui::SameLine();
                }
                else {
                    colPos = 0;
                }
            }

            ImGui::End();
            
            


        }

    }
    /*!*************************************************************************
   Logger, can toggle between types of messages you want to view
   ****************************************************************************/
    void LevelEditor::Logger() const
    {
        if (is_ShowWindow)
        {

            if (ImGui::Begin("Logger"))
            {
                static bool info = true;
                ImGui::Checkbox("Info       ", &info);

                ImGui::SameLine();

                static bool warning = true;
                ImGui::Checkbox("warning    ", &warning);

                ImGui::SameLine();

                static bool error = true;
                ImGui::Checkbox("error", &error);

                static bool system = true;
                ImGui::Checkbox("system     ", &system);

                ImGui::SameLine();

                static bool log = true;
                ImGui::Checkbox("log        ", &log);

                ImGui::SameLine();

                static bool fatal = true;
                ImGui::Checkbox("fatal", &fatal);

                if (info)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                    ImGui::Text(Log::GetImguiLog().c_str());
                    ImGui::PopStyleColor();



                }

                if (warning)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
                    ImGui::Text("This is an Warning Message");
                    ImGui::PopStyleColor();
                }

                if (error)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                    ImGui::Text("This is an Error Message");
                    ImGui::PopStyleColor();
                }

                if (system)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(52, 67, 235, 255));
                    ImGui::Text("This is an System Message");
                    ImGui::PopStyleColor();

                    if (p_Audio->GetPlayingChannels() > 0)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                        ImGui::Text("Audio Channel %d is played", p_Audio->GetPlayingChannels());
                        ImGui::PopStyleColor();
                    }
                }

                if (log)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(52, 222, 235, 255));
                    ImGui::Text("This is an Log Message");
                    ImGui::PopStyleColor();
                }

                if (fatal)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                    ImGui::Text("This is an Fatal Message");
                    ImGui::PopStyleColor();

                }
            }
            ImGui::End();
        }
    }

    /*!*************************************************************************
   Docking allows us to dock the ImGui windows to the edges of the scene
   ****************************************************************************/
    void LevelEditor::Docking()
    {
        //Set docking flags
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace", 0, window_flags);

        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::End();

    }


    /*!*************************************************************************
    Profiler displays information about the users system such as

    Renderer and version of renderer (GPU)

    FPS

    Rendered info e.g. Quads, vertices, indices drawn etc.
    ****************************************************************************/
    void LevelEditor::Profiler()
    {
        if (is_ShowWindow)
        {
            ImGui::Begin("Profiler");
            //Opengl information
            ImGui::Text("Opengl Information ");
            ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
            ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
            ImGui::Text("Version: %s", glGetString(GL_VERSION));
            ImGui::Text("\n");

            // how much time per frame and Fps
            ImGui::Text("Application average % .3f ms / frame(% .3f FPS)", 1000.0f / FramePerSec::GetInstance().GetFps(), FramePerSec::GetInstance().GetFps());
            ImGui::Text("GUI mouse coordinates: %.3f %.3f", p_GUI->MousePosition.x, p_GUI->MousePosition.y);

            //Renderering Information
            ImGui::Text("\n");
            auto Infos = Renderer::GetInfo();
            ImGui::Text("Renderer Information");
            ImGui::Text("Draw Calls: %d", Infos.n_DrawCalls);
            ImGui::Text("Quads: %d", Infos.n_Quad);
            ImGui::Text("Vertices: %d", Infos.TotalVertexUsed());
            ImGui::Text("Indices: %d", Infos.TotalIndexUsed());

            ImGui::Checkbox("Show physics colliders", &mDebugDraw);

            mSystemRunTime[0] = Timer::GetInstance().GetDT(Systems::COLLISION) / Timer::GetInstance().GetGlobalDT();
            mSystemRunTime[1] = Timer::GetInstance().GetDT(Systems::GRAPHIC) / Timer::GetInstance().GetGlobalDT();
            mSystemRunTime[2] = Timer::GetInstance().GetDT(Systems::PHYSICS) / Timer::GetInstance().GetGlobalDT();
            mSystemRunTime[3] = Timer::GetInstance().GetDT(Systems::LOGIC) / Timer::GetInstance().GetGlobalDT();
            mSystemRunTime[4] = Timer::GetInstance().GetDT(Systems::API) / Timer::GetInstance().GetGlobalDT();

            ImGui::Text("COLLISION");
            ImGui::SameLine(80);
            ImGui::ProgressBar(mSystemRunTime[0], ImVec2(-1.0f, 0.0f));
            ImGui::Text("GRAPHIC");
            ImGui::SameLine(80);
            ImGui::ProgressBar(mSystemRunTime[1], ImVec2(-1.0f, 0.0f));
            ImGui::Text("PHYSICS");
            ImGui::SameLine(80);
            ImGui::ProgressBar(mSystemRunTime[2], ImVec2(-1.0f, 0.0f));
            ImGui::Text("LOGIC");
            ImGui::SameLine(80);
            ImGui::ProgressBar(mSystemRunTime[3], ImVec2(-1.0f, 0.0f));
            ImGui::Text("API");
            ImGui::SameLine(80);
            ImGui::ProgressBar(mSystemRunTime[4], ImVec2(-1.0f, 0.0f));
            ImGui::Text("\n");

			// Custom Memory Manager Info
			ImGui::Text("Memory Manager Info");
			ImGui::Text("Memory Manager is %s", p_MemoryManager.IsEnabled() ? "Enabled" : "Disabled");
            ImGui::Text("Memory Block Size");
            ImGui::SameLine();
            ImGui::DragInt("##BlockSize", (int*)&mMemoryBlock, 1.0f, 1, 256);
            ImGui::Text("Memory Blocks per Page");
            ImGui::SameLine();
            ImGui::DragInt("##BlocksPerPage", (int*)&mMemoryPage, 1.0f, 1, 128);

            if (p_MemoryManager.IsEnabled()) 
            {
                if (ImGui::Button("Apply##MemorySettings"))
                {
					mMemoryBlock = mMemoryBlock < 1 ? kDefaultBlockSize : mMemoryBlock > kDefaultBlockSize ? kDefaultBlockSize : mMemoryBlock;
					mMemoryPage = mMemoryPage < 1 ? kDefaultBlocksPerPage : mMemoryPage > kDefaultBlocksPerPage ? kDefaultBlocksPerPage : mMemoryPage;

                    p_MemoryManager.SetBlockSize(static_cast<size_t>(mMemoryBlock));
                    p_MemoryManager.SetBlocksPerPage(static_cast<size_t>(mMemoryPage));
                    p_MemoryManager.Configure(p_MemoryManager.GetBlockSize(), p_MemoryManager.GetBlocksPerPage());
                }
                ImGui::Text("Memory Block Size: %zu bytes", p_MemoryManager.GetBlockSize());
                ImGui::Text("Memory Blocks per Page: %zu", p_MemoryManager.GetBlocksPerPage());
			    ImGui::Text("Total Memory Allocated: %zu bytes", p_MemoryManager.GetTotalMemoryAllocated());
			    ImGui::Text("Total Memory Used: %zu bytes", p_MemoryManager.GetTotalMemoryUsed());
				ImGui::ProgressBar(static_cast<float>(p_MemoryManager.GetTotalMemoryUsed()) / static_cast<float>(p_MemoryManager.GetTotalMemoryAllocated()), ImVec2(-1.0f, 0.0f));
				ImGui::Text("Free Block Count: %zu", p_MemoryManager.GetFreeBlocks());
            }
            else
            {
				ImGui::Text("Press F9 to enable Custom Memory Manager");
            }
            ImGui::End();
        }
    }



    void LevelEditor::Layering() const
    {
        if (!is_ShowWindow) return;

        ImGui::Begin("Layering");

        ImGui::Text("Layering System (Only shows used layers)");
        ImGui::Text("Top draws first (behind). Bottom draws last (in front).");
        ImGui::Separator();

        std::vector<int> usedLayers;
        usedLayers.reserve(NocturnalEngine::Graphic::MAX_LAYERS);

        for (Entity e = 0; e < p_ecs.GetTotalSceneEntities(); ++e)
        {
            if (!p_ecs.HaveComponent<Sprite>(e)) continue;

            int l = p_ecs.GetComponent<Sprite>(e).LayerOrder;
            if (l < 0) l = 0;
            if (l >= NocturnalEngine::Graphic::MAX_LAYERS) l = NocturnalEngine::Graphic::MAX_LAYERS - 1;

            usedLayers.push_back(l);
            if (p_ecs.HaveComponent<Text>(e)) {
                int text_layer_order = p_ecs.GetComponent<Text>(e).Get_layering_order();
                usedLayers.push_back(text_layer_order);
            }
            
        }

        if (usedLayers.empty())
            usedLayers.push_back(0);

        std::sort(usedLayers.begin(), usedLayers.end());
        usedLayers.erase(std::unique(usedLayers.begin(), usedLayers.end()), usedLayers.end());

        std::vector<int> visibleLayers;
        visibleLayers.reserve(usedLayers.size());

        for (int i = 0; i < NocturnalEngine::Graphic::MAX_LAYERS; ++i)
        {
            int layer = NocturnalEngine::Graphic::layerDrawOrder[i];

            if (std::binary_search(usedLayers.begin(), usedLayers.end(), layer))
                visibleLayers.push_back(layer);
        }

        for (int visIdx = 0; visIdx < (int)visibleLayers.size(); ++visIdx)
        {
            int layer = visibleLayers[visIdx];
            ImGui::PushID(visIdx);

            ImGui::Checkbox("##enable", &NocturnalEngine::Graphic::layerEnabled[layer]);
            ImGui::SameLine();

            std::string label = "Layer " + std::to_string(layer);
            ImGui::Selectable(label.c_str(), false);

            if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("LAYER_REORDER_VISIBLE", &visIdx, sizeof(int));
                ImGui::Text("Move %s", label.c_str());
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LAYER_REORDER_VISIBLE"))
                {
                    int srcVisIdx = *(const int*)payload->Data;
                    int dstVisIdx = visIdx;

                    if (srcVisIdx != dstVisIdx &&
                        srcVisIdx >= 0 && srcVisIdx < (int)visibleLayers.size() &&
                        dstVisIdx >= 0 && dstVisIdx < (int)visibleLayers.size())
                    {
                        int movingLayer = visibleLayers[srcVisIdx];
                        int dstLayer = visibleLayers[dstVisIdx];

                        int srcGlobal = -1, dstGlobal = -1;
                        for (int g = 0; g < NocturnalEngine::Graphic::MAX_LAYERS; ++g)
                        {
                            if (NocturnalEngine::Graphic::layerDrawOrder[g] == movingLayer) srcGlobal = g;
                            if (NocturnalEngine::Graphic::layerDrawOrder[g] == dstLayer)    dstGlobal = g;
                        }

                        if (srcGlobal != -1 && dstGlobal != -1)
                        {
                            int temp = NocturnalEngine::Graphic::layerDrawOrder[srcGlobal];

                            if (srcGlobal < dstGlobal)
                            {
                                for (int k = srcGlobal; k < dstGlobal; ++k)
                                    NocturnalEngine::Graphic::layerDrawOrder[k] = NocturnalEngine::Graphic::layerDrawOrder[k + 1];
                                NocturnalEngine::Graphic::layerDrawOrder[dstGlobal] = temp;
                            }
                            else
                            {
                                for (int k = srcGlobal; k > dstGlobal; --k)
                                    NocturnalEngine::Graphic::layerDrawOrder[k] = NocturnalEngine::Graphic::layerDrawOrder[k - 1];
                                NocturnalEngine::Graphic::layerDrawOrder[dstGlobal] = temp;
                            }
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::PopID();
        }

        ImGui::End();
    }


    /*!*************************************************************************
  // Create, destroy and clone entities in the hirarchy window
  ****************************************************************************/
    void LevelEditor::Hierarchy()
    {
        if (is_ShowWindow)
        {
            ImGui::Begin("Hierarchy");
            Entity totalEntities = p_ecs.GetTotalSceneEntities();

            if (ImGui::Button("Create Entity") && totalEntities != MAX_ENTITIES)
            {
                Entity newEntity = p_ecs.CreateEntity();
                p_ecs.AddComponent<NameTag>(newEntity, C_NameTagComponent);

                Transform transform{};
                transform.SetPos(0.0f, 0.0f);
                transform.SetScale(1.0f, 1.0f);
                transform.SetRot(0.0f);
                p_ecs.AddComponent<Transform>(newEntity, transform);

                selectedEntity = newEntity;
                PushUndoSnapshotAndInvalidateRedo();
            }

            if (totalEntities > 0)
            {
                ImGui::SameLine();
                if (ImGui::Button("Destroy Entity"))
                {
                    if (selectedEntity != MAX_ENTITIES)
                    {
                        p_Heirarchy.DestroyHierarchy(selectedEntity); // destroys children too
                        selectedEntity = MAX_ENTITIES;
                        PushUndoSnapshotAndInvalidateRedo();
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Clone Entity") && p_ecs.GetTotalSceneEntities() != 0)// there is entity alive
                {
                    if (selectedEntity != MAX_ENTITIES)
                    {
                        Entity cloned = p_Heirarchy.CloneEntity(selectedEntity);
                        selectedEntity = cloned;
                        PushUndoSnapshotAndInvalidateRedo();
                    }
                }
                
                //loops through each alive entity and shows it in the heirarchy. Tree node in the futre in case of parenting
                for (Entity e:p_ecs.GetAliveEntitiesSet())
                {
                    if (p_Heirarchy.IsChild(e)) continue;
                    DrawHierarchyNode(e);
                    

                }

                
            }

            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
                selectedEntity = MAX_ENTITIES;

            // Drop onto empty space = unparent
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY"))
                {
                    Entity dragged = *(Entity*)payload->Data;
                    if (p_Heirarchy.IsChild(dragged))
                    {
                        p_Heirarchy.RemoveParent(dragged);
                        PushUndoSnapshotAndInvalidateRedo();
                    }
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::End();
        }

    }

    void LevelEditor::DrawHierarchyNode(Entity e) {
        if (p_ecs.HaveComponent<NameTag>(e))
        {
            std::string tag = p_ecs.HaveComponent<NameTag>(e)? p_ecs.GetComponent<NameTag>(e).GetNameTag():"";
            bool hasChildren = p_Heirarchy.IsParent(e);

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (!hasChildren)
                flags |= ImGuiTreeNodeFlags_Leaf; // no arrow if no children
            if (selectedEntity == e)
                flags |= ImGuiTreeNodeFlags_Selected;

            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)e, flags, tag.c_str());

            if (ImGui::IsItemClicked())
                selectedEntity = e;

            // Drag source
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                ImGui::SetDragDropPayload("HIERARCHY_ENTITY", &e, sizeof(Entity));
                ImGui::Text("Moving: %s", tag.c_str());
                ImGui::EndDragDropSource();
            }

            // Drop target — parent dragged entity onto this one
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY"))
                {
                    Entity dragged = *(Entity*)payload->Data;
                    if (dragged != e && dragged != MAX_ENTITIES)
                    {
                        // Prevent parenting to own descendant
                        bool isDescendant = false;
                        std::function<void(Entity)> checkDesc = [&](Entity parent)
                            {
                                for (Entity child : p_Heirarchy.GetChildren(parent))
                                {
                                    if (child == e) { isDescendant = true; return; }
                                    checkDesc(child);
                                }
                            };
                        checkDesc(dragged);

                        if (!isDescendant)
                        {
                            p_Heirarchy.SetParent(dragged, e);
                            PushUndoSnapshotAndInvalidateRedo();
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (ImGui::BeginPopupContextItem())
            {

                ImGui::Text("Create Prefab");
                ImGui::InputText("Name (optional)", prefabNameBuffer, sizeof(prefabNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

                if (ImGui::Button("Create"))
                {
                    std::string prefabName = prefabNameBuffer;
                    if (p_PrefabManager.CloneToList(e, prefabName))std::cout << "Prefab cloned\n";

                    prefabNameBuffer[0] = '\0'; // clear buffer
                    ImGui::CloseCurrentPopup();
                }

                ImGui::Separator();

                if (p_Heirarchy.IsChild(e))
                {
                    if (ImGui::MenuItem("Unparent"))
                    {
                        p_Heirarchy.RemoveParent(e);
                        PushUndoSnapshotAndInvalidateRedo();
                    }
                }
                ImGui::EndPopup();
            }

            if (opened) {
                for (Entity child : p_Heirarchy.GetChildren(e))
                    DrawHierarchyNode(child);
                ImGui::TreePop();
            }
            ImGui::Separator();
        }
    }
    /*!*************************************************************************
   Inspector allows us to manipulate the entity properties, modifying scale,
   rotation and position of the object.
   ****************************************************************************/
    void LevelEditor::Inspector()
    {
        if (is_ShowWindow)
        {
            ImGui::Begin("Inspector");
            if (selectedEntity < MAX_ENTITIES && selectedEntity >= 0 && p_ecs.GetTotalSceneEntities() != 0)// if the selectedEntityExist
            {
                //create component for the selected entity 
                if (ImGui::Button("Add Component"))
                    ImGui::OpenPopup("Add Component");

                //Add component section, this is a popup
                if (ImGui::BeginPopup("Add Component"))
                {
                    ImGui::SeparatorText("Engine Components");

                    //iterates through every component registered into the engine's component registry
                    for (std::pair<const std::string, const ComponentRegistry::ComponentDescriptor> compdesc:p_ECompReg.GetAll())
                    {
                        if (ImGui::MenuItem(compdesc.first.c_str()))
                        {
                            if (!compdesc.second.hasFunc(selectedEntity)) //if the entity doesnt have the component
                            {
                                compdesc.second.addFunc(selectedEntity); //this calls the adder function
                                PushUndoSnapshotAndInvalidateRedo();
                            }
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    
                    //if
                    if (mgameComponents)
                    {
                        // Add a separator line here to seperate the game components from engine components
                        const std::unordered_map<std::string, ComponentRegistry::ComponentDescriptor>& gameCompsMap = mgameComponents->GetAll();
                        if(!gameCompsMap.empty())ImGui::SeparatorText("Game Components");

                        //iterates through all the game components
                        for (std::pair<const std::string, const ComponentRegistry::ComponentDescriptor> compdesc : gameCompsMap)
                        {
                            if (ImGui::MenuItem(compdesc.first.c_str()))
                            {
                                if (!compdesc.second.hasFunc(selectedEntity))
                                {
                                    compdesc.second.addFunc(selectedEntity);
                                    PushUndoSnapshotAndInvalidateRedo();
                                }
                                ImGui::CloseCurrentPopup();
                            }
                        }
                    }
                    ImGui::EndPopup();
                }

                bool inspectorChanged = false;
                //iterates through all the engine's game components and displays them
                for (std::pair<const std::string, const ComponentRegistry::ComponentDescriptor> compdesc:p_ECompReg.GetAll())
                {
                    if (compdesc.second.hasFunc(selectedEntity))
                    {
                        inspectorChanged |= compdesc.second.displayInEditor(selectedEntity);
                    }
                }

                //iterates through all the game components and shows them 
                if (mgameComponents)
                {
                    for (std::pair<const std::string, const ComponentRegistry::ComponentDescriptor> compdesc : mgameComponents->GetAll())
                    {
                        if (compdesc.second.hasFunc(selectedEntity))
                        {
                            inspectorChanged |= compdesc.second.displayInEditor(selectedEntity);
                        }
                    }
                }

                if (inspectorChanged)
                    mInspectorEditDirty = true;

                if (mInspectorEditDirty && !ImGui::IsAnyItemActive())
                {
                    PushUndoSnapshotAndInvalidateRedo();
                    mInspectorEditDirty = false;
                }

                if (ImGui::Button("Delete Component"))
                    ImGui::OpenPopup("Delete Component");

                if (ImGui::BeginPopup("Delete Component"))
                {
                    ImGui::SeparatorText("Engine Components");
                    for (std::pair<const std::string, const ComponentRegistry::ComponentDescriptor> compdesc : p_ECompReg.GetAll())
                    {
                        if (ImGui::MenuItem(compdesc.first.c_str()))
                        {
                            if (compdesc.second.hasFunc(selectedEntity))
                            {
                                compdesc.second.removeFunc(selectedEntity);
                                PushUndoSnapshotAndInvalidateRedo();
                            }
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    if (mgameComponents)
                    {
                        // Add a separator line here to seperate the game components from engine components
                        const std::unordered_map<std::string, ComponentRegistry::ComponentDescriptor>& gameCompsMap = mgameComponents->GetAll();
                        if (!gameCompsMap.empty())ImGui::SeparatorText("Game Components");
                        for (std::pair<const std::string, const ComponentRegistry::ComponentDescriptor> compdesc : gameCompsMap)
                        {
                            if (ImGui::MenuItem(compdesc.first.c_str()))
                            {
                                if (compdesc.second.hasFunc(selectedEntity))
                                {
                                    compdesc.second.removeFunc(selectedEntity);
                                    PushUndoSnapshotAndInvalidateRedo();
                                }
                                ImGui::CloseCurrentPopup();
                            }
                        }
                    }
                    ImGui::EndPopup();
                }
            }
            ImGui::End();
        } //end of is_ShowWindow

    }



    /*!*************************************************************************
     Audio manager allows users to select and play, pause and test different
     audios in the editor
    ****************************************************************************/
    void LevelEditor::AudioEditor()
    {
        if (is_ShowWindow)
        {
            static bool audioComponentEditDirty = false;
            std::vector<const char*> audiofilenames;
            const size_t arraysize = 100;
            const char* audioFileList[arraysize];
            std::string audioPath = "Assets/Audio/";
            static int currentfile = 0;

            for (auto& item : mAudioFile)
            {
                audiofilenames.push_back(item.c_str());
            }

            std::copy(audiofilenames.begin(), audiofilenames.end(), audioFileList);

            for (size_t i = audiofilenames.size(); i < arraysize; i++)
            {
                audioFileList[i] = "EMPTY";
            }

            ImGui::Begin("Audio Manager");

            ImGui::Combo("##LoadFile", &currentfile, audioFileList, static_cast<int>(audiofilenames.size()), static_cast<int>(audiofilenames.size()));

            static bool pauseButton = false;

            if (ImGui::Button("Play Audio"))
            {
                ImGui::SameLine();
                ImGui::Text("Playing!");
                current_sound = p_Audio->PlaySound(audioPath + mAudioFileList[currentfile].path().filename().string(), Audio::AudioType::MASTER);
            }

            for (auto i = p_Audio->mChannelMap.begin(); i != p_Audio->mChannelMap.end(); i++)
            {
                //print out text indicating channel number
                ImGui::Text("Playing Channel %s", std::to_string(i->first).c_str());

                if (ImGui::Button("Pause Audio"))
                {
                    p_Audio->PauseSound(i->first);
                }

                ImGui::SameLine();

                if (ImGui::Button("Unpause Audio"))
                {
                    p_Audio->UnpauseSound(i->first);
                }

                if (ImGui::Button("Stop Audio"))
                {
                    p_Audio->StopChannel(i->first);
                }

            }

            if (ImGui::Button("Delete Audio"))
            {
                std::filesystem::remove(mAudioFileList[currentfile].path());
                mAudioFileList.erase(mAudioFileList.begin() + currentfile);
                // used to load audio
                mAudioFile.erase(mAudioFile.begin() + currentfile);
            }

            static int item_current = 1;
            UNREFERENCED_PARAMETER(item_current);
            bool hasAudioComponent = false;
            bool audioComponentChanged = false;

            float f1 = p_Audio->GetVolumeByChannel(p_Audio->GetMasterChannelGroup());
            float f4 = p_Audio->GetVolumeByChannel(p_Audio->GetBGMChannelGroup());
            float f5 = p_Audio->GetVolumeByChannel(p_Audio->GetSFXChannelGroup());
            
            for (Entity i = 0; i < p_ecs.GetTotalSceneEntities(); ++i) 
            {
                if (p_ecs.HaveComponent<Audio>(i)) 
                {
                    hasAudioComponent = true;
                    f1 = p_ecs.GetComponent<Audio>(i).GetMasterVolume();
                    f4 = p_ecs.GetComponent<Audio>(i).GetBGMVolume();
                    f5 = p_ecs.GetComponent<Audio>(i).GetSFXVolume();
                }
            }
            //set voulume slider
            audioComponentChanged |= ImGui::SliderFloat("Master Volume", &f1, 0.0f, 1.0f, "Min - Max %.3f");
            p_Audio->SetVolumeByChannel(p_Audio->GetMasterChannelGroup(), f1);

            audioComponentChanged |= ImGui::SliderFloat("BGM vol", &f4, 0.0f, 1.0f, "Min - Max %.3f");
            if (f4 > f1)
            {
                f4 = f1;
            }
            p_Audio->SetVolumeByChannel(p_Audio->GetBGMChannelGroup(), f4);


            audioComponentChanged |= ImGui::SliderFloat("SFX vol", &f5, 0.0f, 1.0f, "Min - Max %.3f");
            if (f5 > f1)
            {
                f5 = f1;
            }
            p_Audio->SetVolumeByChannel(p_Audio->GetSFXChannelGroup(), f5);

            for (Entity i = 0; i < p_ecs.GetTotalSceneEntities(); ++i)
            {
                if (p_ecs.HaveComponent<Audio>(i))
                {
                    p_ecs.GetComponent<Audio>(i).SetMasterVolume(f1);
                    p_ecs.GetComponent<Audio>(i).SetBGMVolume(f4);
                    p_ecs.GetComponent<Audio>(i).SetSFXVolume(f5);
                }
            }

            // Audio Effects (DSP) controls
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Audio Effects (DSP)"))
            {
                static AudioEffectSettings editorFx;
                static bool fxInitialized = false;

                // Sync from first Audio component found in scene on first open
                if (!fxInitialized)
                {
                    for (Entity ei = 0; ei < p_ecs.GetTotalSceneEntities(); ++ei)
                    {
                        if (p_ecs.HaveComponent<Audio>(ei))
                        {
                            editorFx = p_ecs.GetComponent<Audio>(ei).GetEffectSettings();
                            break;
                        }
                    }
                    editorFx = p_Audio->GetCurrentEffects();
                    fxInitialized = true;
                }

                // Low-pass filter
                audioComponentChanged |= ImGui::Checkbox("Low-Pass Filter##Editor", &editorFx.lowpassEnabled);
                if (editorFx.lowpassEnabled)
                {
                    audioComponentChanged |= ImGui::SliderFloat("LP Cutoff (Hz)##Editor", &editorFx.lowpassCutoff, 10.0f, 22000.0f, "%.0f");
                    audioComponentChanged |= ImGui::SliderFloat("LP Resonance##Editor", &editorFx.lowpassResonance, 1.0f, 10.0f, "%.1f");
                }

                // High-pass filter
                audioComponentChanged |= ImGui::Checkbox("High-Pass Filter##Editor", &editorFx.highpassEnabled);
                if (editorFx.highpassEnabled)
                {
                    audioComponentChanged |= ImGui::SliderFloat("HP Cutoff (Hz)##Editor", &editorFx.highpassCutoff, 10.0f, 22000.0f, "%.0f");
                    audioComponentChanged |= ImGui::SliderFloat("HP Resonance##Editor", &editorFx.highpassResonance, 1.0f, 10.0f, "%.1f");
                }

                // Echo / Delay
                audioComponentChanged |= ImGui::Checkbox("Echo / Delay##Editor", &editorFx.echoEnabled);
                if (editorFx.echoEnabled)
                {
                    audioComponentChanged |= ImGui::SliderFloat("Echo Delay (ms)##Editor", &editorFx.echoDelay, 1.0f, 5000.0f, "%.0f");
                    audioComponentChanged |= ImGui::SliderFloat("Echo Feedback (%%)##Editor", &editorFx.echoFeedback, 0.0f, 100.0f, "%.0f");
                    audioComponentChanged |= ImGui::SliderFloat("Echo Dry (dB)##Editor", &editorFx.echoDryLevel, -80.0f, 10.0f, "%.1f");
                    audioComponentChanged |= ImGui::SliderFloat("Echo Wet (dB)##Editor", &editorFx.echoWetLevel, -80.0f, 10.0f, "%.1f");
                }

                // Reverb
                audioComponentChanged |= ImGui::Checkbox("Reverb##Editor", &editorFx.reverbEnabled);
                if (editorFx.reverbEnabled)
                {
                    audioComponentChanged |= ImGui::SliderFloat("Decay Time (ms)##Editor", &editorFx.reverbDecayTime, 100.0f, 20000.0f, "%.0f");
                    audioComponentChanged |= ImGui::SliderFloat("Early Delay (ms)##Editor", &editorFx.reverbEarlyDelay, 0.0f, 300.0f, "%.1f");
                    audioComponentChanged |= ImGui::SliderFloat("Late Delay (ms)##Editor", &editorFx.reverbLateDelay, 0.0f, 100.0f, "%.1f");
                    audioComponentChanged |= ImGui::SliderFloat("HF Reference (Hz)##Editor", &editorFx.reverbHFReference, 20.0f, 20000.0f, "%.0f");
                    audioComponentChanged |= ImGui::SliderFloat("Diffusion (%%)##Editor", &editorFx.reverbDiffusion, 0.0f, 100.0f, "%.0f");
                    audioComponentChanged |= ImGui::SliderFloat("Density (%%)##Editor", &editorFx.reverbDensity, 0.0f, 100.0f, "%.0f");
                    audioComponentChanged |= ImGui::SliderFloat("Reverb Wet (dB)##Editor", &editorFx.reverbWetLevel, -80.0f, 20.0f, "%.1f");
                    audioComponentChanged |= ImGui::SliderFloat("Reverb Dry (dB)##Editor", &editorFx.reverbDryLevel, -80.0f, 20.0f, "%.1f");
                }

                // Apply effects to audio engine in real-time
                p_Audio->ApplyEffects(editorFx);

                // Propagate editor effect settings back to all Audio components in the scene
                for (Entity ei = 0; ei < p_ecs.GetTotalSceneEntities(); ++ei)
                {
                    if (p_ecs.HaveComponent<Audio>(ei))
                    {
                        p_ecs.GetComponent<Audio>(ei).SetEffectSettings(editorFx);
                    }
                }
            }

            if (hasAudioComponent && audioComponentChanged)
                audioComponentEditDirty = true;

            if (audioComponentEditDirty && !ImGui::IsAnyItemActive())
            {
                PushUndoSnapshotAndInvalidateRedo();
                audioComponentEditDirty = false;
            }

            ImGui::End();
        }
    }

    void LevelEditor::AnimationEditor()
    {
        static bool animationEditDirty = false;
        if (!is_ShowWindow)
            return;

        ImGui::Begin("Animation Editor");

        if (selectedEntity == MAX_ENTITIES || !p_ecs.HaveComponent<Sprite>(selectedEntity))
        {
            ImGui::Text("Select an entity with a Sprite component.");
            ImGui::End();
            return;
        }

        Sprite& sprite = p_ecs.GetComponent<Sprite>(selectedEntity);

        bool changed = false;
        changed |= ImGui::Checkbox("Animated", &sprite.is_Animated);

        if (sprite.Get_SPRITE_type() == Sprite::sprite_type::texture) {
            // Animated is the single source of truth. Keep legacy field synced for compatibility.
            if (sprite.is_SpriteSheet != sprite.is_Animated)
            {
                sprite.is_SpriteSheet = sprite.is_Animated;
                changed = true;
            }

            const std::string& textureName = sprite.GetTexture();
            ImGui::Text("Current Sprite Texture: %s", textureName.c_str());

            if (!sprite.is_Animated)
            {
                ImGui::Text("Enable Animated to edit animation settings.");
                ImGui::End();
                return;
            }

            if (!sprite.stateManagerEnabled)
            {
                ImGui::Separator();
                ImGui::Text("Spritesheet UV");
                changed |= ImGui::DragFloat("U", &sprite.GetUVCoor().x, 0.5f);
                changed |= ImGui::DragFloat("V", &sprite.GetUVCoor().y, 0.5f);
            }

            ImGui::Separator();
            ImGui::Text("Animation State Manager");
            changed |= ImGui::Checkbox("Enable State Manager", &sprite.stateManagerEnabled);

            if (sprite.stateManagerEnabled)
            {
                // Shared picker used by each rule to select a texture from loaded assets.
                auto drawStateTexturePicker = [&](const char* label, std::string& textureKey, int id)
                    {
                        ImGui::PushID(id);
                        ImGui::Text("%s", label); ImGui::SameLine();
                        ImGui::SetNextItemWidth(180.0f);
                        if (ImGui::BeginCombo("##StateTexture", textureKey.c_str()))
                        {
                            for (auto& [str, tex] : ResourceManager::textures)
                            {
                                UNREFERENCED_PARAMETER(tex);
                                if (ImGui::Selectable(str.c_str()))
                                {
                                    textureKey = str;
                                    changed = true;
                                }
                            }
                            ImGui::EndCombo();
                        }

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Textures"))
                            {
                                textureKey = *(const std::string*)payload->Data;
                                changed = true;
                            }
                            ImGui::EndDragDropTarget();
                        }
                        ImGui::PopID();
                    };

                std::vector<std::string> previewLabels{};
                previewLabels.emplace_back("Auto");
                // Preview options mirror the current rule list.
                for (size_t i = 0; i < sprite.stateRules.size(); ++i)
                {
                    if (sprite.stateRules[i].name.empty())
                        previewLabels.emplace_back("State " + std::to_string(i + 1));
                    else
                        previewLabels.push_back(sprite.stateRules[i].name);
                }
                std::vector<const char*> previewItems{};
                previewItems.reserve(previewLabels.size());
                for (const std::string& lbl : previewLabels)
                    previewItems.push_back(lbl.c_str());
                if (!previewItems.empty())
                    changed |= ImGui::Combo("Preview State", &sprite.statePreviewMode, previewItems.data(), static_cast<int>(previewItems.size()));

                if (ImGui::Button("Add State"))
                {
                    // New rules default to "Always" and current texture for quick setup.
                    Sprite::AnimationStateRule r{};
                    r.name = "State " + std::to_string(sprite.stateRules.size() + 1);
                    r.textureKey = sprite.GetTexture();
                    r.uSize = sprite.GetUVCoor().x;
                    r.vSize = sprite.GetUVCoor().y;
                    r.source = 3;
                    r.compare = 0;
                    r.threshold = 0.5f;
                    r.priority = 0;
                    r.enabled = true;
                    sprite.stateRules.push_back(r);
                    changed = true;
                }

                int deleteIndex = -1;
                for (size_t i = 0; i < sprite.stateRules.size(); ++i)
                {
                    auto& rule = sprite.stateRules[i];
                    ImGui::PushID(static_cast<int>(i) + 5100);

                    std::string header = "Rule " + std::to_string(i + 1) + ": " +
                        (rule.name.empty() ? "Unnamed" : rule.name) +
                        "###AnimStateRule" + std::to_string(i);
                    if (ImGui::TreeNode(header.c_str()))
                    {
                        changed |= ImGui::Checkbox("Enabled", &rule.enabled);

                        char nameBuf[64]{};
                        strncpy(nameBuf, rule.name.c_str(), sizeof(nameBuf) - 1);
                        if (ImGui::InputText("State Name", nameBuf, sizeof(nameBuf)))
                        {
                            rule.name = nameBuf;
                            changed = true;
                        }

                        drawStateTexturePicker("Texture", rule.textureKey, 6000 + static_cast<int>(i));
                        changed |= ImGui::DragFloat("U", &rule.uSize, 0.5f, 1.0f, 4096.0f, "%.1f");
                        changed |= ImGui::DragFloat("V", &rule.vSize, 0.5f, 1.0f, 4096.0f, "%.1f");

                        const char* sourceItems[] = { "Hunger", "Health", "Sanity", "Always" };
                        changed |= ImGui::Combo("Source", &rule.source, sourceItems, IM_ARRAYSIZE(sourceItems));

                        const char* compareItems[] = { "<= Threshold", ">= Threshold" };
                        changed |= ImGui::Combo("Condition", &rule.compare, compareItems, IM_ARRAYSIZE(compareItems));

                        if (rule.source != 3)
                            changed |= ImGui::SliderFloat("Threshold", &rule.threshold, 0.0f, 1.0f, "%.2f");
                        else
                            ImGui::TextDisabled("Always: threshold ignored");

                        changed |= ImGui::DragInt("Priority", &rule.priority, 1.0f);

                        if (ImGui::Button("Delete State"))
                            deleteIndex = static_cast<int>(i);

                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }

                if (deleteIndex >= 0 && deleteIndex < static_cast<int>(sprite.stateRules.size()))
                {
                    // Delete after the loop to avoid invalidating references while drawing.
                    sprite.stateRules.erase(sprite.stateRules.begin() + deleteIndex);
                    if (sprite.statePreviewMode > static_cast<int>(sprite.stateRules.size()))
                        sprite.statePreviewMode = 0;
                    changed = true;
                    
                    ImGui::EndDragDropTarget();
                }

                // Logic scripts do not run while editor is active, so apply preview texture directly here.
                if (sprite.statePreviewMode > 0 &&
                    sprite.statePreviewMode <= static_cast<int>(sprite.stateRules.size()))
                {
                    if (Animation::ApplyStateRule(sprite, sprite.statePreviewMode - 1))
                        changed = true;
                }
            }

            ImGui::Separator();
            ImGui::Text("Spritesheet Timing Editor");

            // Build editable spritesheet list from the active sprite texture + state-manager rules.
            std::vector<std::string> editableTextures{};
            auto appendUniqueTexture = [&](const std::string& key)
                {
                    if (key.empty())
                        return;
                    if (std::find(editableTextures.begin(), editableTextures.end(), key) == editableTextures.end())
                        editableTextures.push_back(key);
                };
            appendUniqueTexture(textureName);
            if (sprite.stateManagerEnabled)
            {
                for (const auto& rule : sprite.stateRules)
                    appendUniqueTexture(rule.textureKey);
            }

            if (editableTextures.empty())
            {
                ImGui::TextDisabled("No spritesheets available to edit.");
            }
            else
            {
                static int selectedTimingSheet = 0;
                if (selectedTimingSheet >= static_cast<int>(editableTextures.size()))
                    selectedTimingSheet = 0;

                std::vector<const char*> timingItems{};
                timingItems.reserve(editableTextures.size());
                for (const auto& key : editableTextures)
                    timingItems.push_back(key.c_str());

                changed |= ImGui::Combo("Timing Target", &selectedTimingSheet, timingItems.data(), static_cast<int>(timingItems.size()));
                const std::string targetTexture = editableTextures[selectedTimingSheet];
                ImGui::Text("Editing Texture: %s", targetTexture.c_str());
                if (ImGui::Button("Set Current Sprite To Timing Target"))
                {
                    if (ResourceManager::GetTexture(targetTexture))
                    {
                        sprite.SetTexture(targetTexture);
                        int x = 0;
                        int y = sprite.GetIndex().y;
                        sprite.SetIndex(x, y);
                        sprite.internaltimer = 0.0f;
                        changed = true;
                    }
                }

                int maxFrames = 1;
                auto texIt = ResourceManager::textures.find(targetTexture);
                if (texIt != ResourceManager::textures.end() && texIt->second)
                {
                    const int texFrames = static_cast<int>(texIt->second->GetWidth() / 512);
                    if (texFrames > 0)
                        maxFrames = texFrames;
                }

                auto& targetTimes = Animation::spriteContainer[targetTexture];
                if (targetTimes.empty())
                {
                    // If no data exists yet, bootstrap from current sprite timings or default values.
                    if (targetTexture == textureName && !sprite.GetDisplayTime().empty())
                        targetTimes = sprite.GetDisplayTime();
                    else
                        targetTimes.resize(maxFrames, 0.1f);
                }

                ImGui::Text("Detected Frames: %d", maxFrames);
                if (ImGui::Button("Sync Frames To Texture Width"))
                {
                    targetTimes.resize(maxFrames > 0 ? maxFrames : 1, 0.1f);
                    changed = true;
                }

                for (int i = 0; i < static_cast<int>(targetTimes.size()); ++i)
                {
                    std::string label = "Frame " + std::to_string(i);
                    if (ImGui::DragFloat(label.c_str(), &targetTimes[i], 0.005f, 0.0f, 10.0f, "%.3f s"))
                        changed = true;
                }

                // Keep sprite-local timing in sync when editing the currently assigned sprite texture.
                if (targetTexture == textureName)
                {
                    sprite.GetDisplayTime() = targetTimes;
                    sprite.GetMaxIndex() = static_cast<int>(targetTimes.size());
                }
            }

            if (changed)
                animationEditDirty = true;

            if (animationEditDirty && !ImGui::IsAnyItemActive())
            {
                PushUndoSnapshotAndInvalidateRedo();
                animationEditDirty = false;
            }
        }

        else { //here is the animation for shape
            if (sprite.Get_SHAPE_type() == Sprite::shape_type::rectangle && sprite.is_Animated) {
                ImGui::PushItemWidth(100.0f);
                ImGui::Text("length of dashed lines"); ImGui::SameLine();
                ImGui::DragFloat("##length of dashed lines", &sprite.line_len, 0.001f, 0.001f, 0.1f);
                sprite.line_len = std::clamp(sprite.line_len, 0.001f, 0.1f);

                ImGui::Text("distance between each dashed lines"); ImGui::SameLine();
                ImGui::DragFloat("##distance between each dashed lines", &sprite.gap_len, 0.001f, 0.0f, 0.1f);
                sprite.gap_len = std::clamp(sprite.gap_len, 0.001f, 0.1f);

                ImGui::Text("movement speed of dashed lines"); ImGui::SameLine();
                ImGui::DragFloat("##movement speed of dashed lines", &sprite.line_mvspeed, 0.01f, -1.0f, 1.0f);
                sprite.line_mvspeed = std::clamp(sprite.line_mvspeed, -1.0f, 1.0f);
            }
        }

        ImGui::End();
    }

    void LevelEditor::HandleUndoRedoShortcuts()
    {
        ImGuiIO& io = ImGui::GetIO();
        // Avoid editor shortcuts while typing in text fields.
        if (io.WantTextInput)
            return;

        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false))
        {
            if (io.KeyShift)
                ApplyRedo();
            else if (mUndoStack.size() > 1)
                ApplyUndo();
        }
        else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false))
        {
            ApplyRedo();
        }
    }

    std::string LevelEditor::CreateSnapshotFilePath() const
    {
        // Counter avoids filename collisions in the same microsecond.
        static std::uint64_t snapshotCounter = 0;
        const auto now = std::chrono::steady_clock::now().time_since_epoch();
        const auto ticks = std::chrono::duration_cast<std::chrono::microseconds>(now).count();
        const std::string filename = "snapshot_" + std::to_string(ticks) + "_" + std::to_string(snapshotCounter++) + ".json";
        return (mUndoDirectory / filename).string();
    }

    void LevelEditor::PushUndoSnapshot()
    {
        const std::string snapshotPath = CreateSnapshotFilePath();
        // Save full scene state to disk.
        if (!p_Scene->SerializeToFile(snapshotPath))
            return;

        mUndoStack.push_back(snapshotPath);
        while (mUndoStack.size() > mMaxUndoSteps)
        {
            // Trim oldest history when cap is exceeded.
            std::filesystem::remove(mUndoStack.front());
            mUndoStack.erase(mUndoStack.begin());
        }
    }

    void LevelEditor::PushUndoSnapshotAndInvalidateRedo()
    {
        // New edits invalidate the redo path.
        PushUndoSnapshot();
        for (const auto& redoSnapshot : mRedoStack)
            std::filesystem::remove(redoSnapshot);
        mRedoStack.clear();
    }

    bool LevelEditor::LoadSnapshot(const std::string& snapshotPath)
    {
        if (!std::filesystem::exists(snapshotPath))
            return false;

        const Entity previousSelection = selectedEntity;

        // Replace world state with snapshot contents.
        if (!p_Scene->DeserializeFromFile(snapshotPath))
            return false;

        if (previousSelection < MAX_ENTITIES &&
            p_ecs.GetAliveEntitiesSet().contains(previousSelection))
        {
            selectedEntity = previousSelection;
        }
        else
        {
            selectedEntity = MAX_ENTITIES;
        }
        return true;
    }

    void LevelEditor::ApplyUndo()
    {
        if (mUndoStack.size() <= 1)
            return;

        const std::string redoSnapshot = mUndoStack.back();
        mUndoStack.pop_back();
        const std::string& undoSnapshot = mUndoStack.back();
        if (!LoadSnapshot(undoSnapshot))
        {
            mUndoStack.push_back(redoSnapshot);
            return;
        }
        mInspectorEditDirty = false;

        mRedoStack.push_back(redoSnapshot);
        while (mRedoStack.size() > mMaxUndoSteps)
        {
            // Enforce history cap.
            std::filesystem::remove(mRedoStack.front());
            mRedoStack.erase(mRedoStack.begin());
        }
    }

    void LevelEditor::ApplyRedo()
    {
        if (mRedoStack.empty())
            return;

        const std::string redoSnapshot = mRedoStack.back();
        mRedoStack.pop_back();
        if (!LoadSnapshot(redoSnapshot))
        {
            mRedoStack.push_back(redoSnapshot);
            return;
        }
        mInspectorEditDirty = false;
        mUndoStack.push_back(redoSnapshot);
        while (mUndoStack.size() > mMaxUndoSteps)
        {
            // Enforce history cap.
            std::filesystem::remove(mUndoStack.front());
            mUndoStack.erase(mUndoStack.begin());
        }
    }

    /*!*************************************************************************
    using std::filesystem we insert the file path if audio assets into the editor
    ****************************************************************************/
    void LevelEditor::insertAudioFilePath(std::string in)
    {
        //create new directory entry
        //insert entry into mAudioFileList
        //insert path into mAudioFile
        auto const& dir_entry = std::filesystem::directory_entry{ in };
        mAudioFileList.emplace_back(dir_entry);
        mAudioFile.emplace_back(dir_entry.path().filename().string());
    }

    /*!*************************************************************************
   using std::filesystem we insert the file path into the system
   ****************************************************************************/
    void LevelEditor::insertTextureFilePath(std::string in)
    {

        if (!std::filesystem::exists(in))
        {
            NOCTURNAL_ENGINE_WARN("Assets/Audio/ folder missing! Creating it...");
            std::filesystem::create_directories(in);
            return;
        }

        auto const& dir_entry = std::filesystem::directory_iterator( in );

        for (const auto& entry : dir_entry)
        {
            if (entry.path().extension() == ".png") {

            }
        }
    }


    /*!*************************************************************************
    This draws the modal which prompts the user to select a path and set a name
    for their new project
    ****************************************************************************/
    void LevelEditor::DrawNewProjectPopup() {
        ImGui::OpenPopup("New Game");

        // Centers the modal
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_Appearing);

        if (ImGui::BeginPopupModal("New Game", &mshowNewProjectPopup))
        {
            ImGui::Text("Create New Game");
            ImGui::Separator();
            ImGui::Spacing();

            // Prompt user to input Project name here
            static char projectName[128] = "";
            ImGui::Text("Game Project Name:");
            ImGui::SetNextItemWidth(-1);
            projNameUpdate=ImGui::InputText("##ProjectName", projectName, sizeof(projectName));

            ImGui::Spacing();

            std::filesystem::path relativePath = std::filesystem::relative(mSelectedProjPath, mBrowsingPath.parent_path());
            std::string pathStr = relativePath.string() + "\\";

            size_t copyLen = std::min(pathStr.size(), sizeof(selectedPathDisplayBuffer) - 1);
            strncpy(selectedPathDisplayBuffer, pathStr.c_str(), copyLen);
            selectedPathDisplayBuffer[copyLen] = '\0';

            // project path, readonly because i am way too lazy to check everytime user types in
            ImGui::Text("Location:");
            ImGui::SetNextItemWidth(-FLT_MIN - 60);
            ImGui::InputText("##ProjectPath", selectedPathDisplayBuffer, sizeof(selectedPathDisplayBuffer),ImGuiInputTextFlags_ReadOnly);
            
            //this opens up the file browser
            ImGui::SameLine();
            if (ImGui::Button("Browse")) { 
                mfileBrowsing = true; 
                currPath.clear();
                currPath = mBrowsingPath;
                
            }


            if (mfileBrowsing)
            {
                StupidFileBrowser(mBrowsingPath,currPath);
            }

            ImGui::Spacing();

            //hopefully to prevent polling 
            if (strlen(projectName) > 0 && projNameUpdate) { 
                fixedProjName = p_ProjMaker.ValidateProjectName({ projectName });
                notInsideAnotherProjFolder = p_ProjMaker.IsNotAnotherGameDirectory(mSelectedProjPath / fixedProjName);
                projNameUpdate = false;
            }

            // Preview full path
            ImGui::TextDisabled("Game Project Path: %s%s", selectedPathDisplayBuffer, fixedProjName.c_str());

            //if we want to modify the root cmake file
            ImGui::Checkbox("Set New Game Project as startup project", &setNewAsStartup);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Validates input
            bool nameIsValid = fixedProjName.length() > 0;
            std::string fullPath = std::string(selectedPathDisplayBuffer) + fixedProjName;
            bool pathExists = std::filesystem::exists(fullPath);

            //checks if inside another project folder (if theres another cmake file in there)
            if (!notInsideAnotherProjFolder)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1)); // red
                ImGui::TextWrapped("Warning: '%s' is inside another Game Project folder!", fullPath.c_str());
                ImGui::PopStyleColor();
            }
            if (pathExists)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1)); // red
                ImGui::TextWrapped("Warning: Game Project '%s' already exists!", fixedProjName.c_str());
                ImGui::PopStyleColor();
            }

            // Create button
            if (!nameIsValid || pathExists||!notInsideAnotherProjFolder)
                ImGui::BeginDisabled();

            if (ImGui::Button("Create Game Project"))
            {
                if (p_ProjMaker.CreateNewProject(fixedProjName,mSelectedProjPath,setNewAsStartup ))
                {
                    mshowNewProjectPopup = false;
                    needCloseToRload = true;
                    memset(projectName, 0, sizeof(projectName));
                }
            }

            if (!nameIsValid || pathExists|| !notInsideAnotherProjFolder)
                ImGui::EndDisabled();

            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
                mshowNewProjectPopup = false;
                memset(projectName, 0, sizeof(projectName));
            }

            ImGui::EndPopup();
        }
    }

    /*!*************************************************************************
    This draws the modal which prompts the user to open a Project. Since the 
    editor is not able to do that yet, this is not implemented
    ****************************************************************************/
    void LevelEditor::DrawOpenProjectPopup() {

    }
    
    /*!*************************************************************************
    Sorry I named this when i was angry this is the file browser used for selecting
    a directory to load the project into. I might chantge it so that if i ever 
    implement the open project function i can use it for both
    ****************************************************************************/
    void LevelEditor::StupidFileBrowser( std::filesystem::path& browsingPath, std::filesystem::path& pathReference, bool isNewProj) {
        ImGui::OpenPopup("Browse");

        // Center the modal
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_Appearing);

        if (ImGui::BeginPopupModal("Browse", &mfileBrowsing))
        {
                
                std::filesystem::path path = browsingPath;
                std::string folderName = path.filename().string();
                if (!isNewProj) {
                    if (ImGui::Button("Up"))
                    {
                        std::filesystem::path absPath = std::filesystem::absolute(browsingPath);

                        if (absPath.has_parent_path())
                        {

                            std::filesystem::path parent = absPath.parent_path();

                            browsingPath = parent;
                        }
                    }
                    ImGui::SameLine();
                }

                ImGui::Text("\\%s", folderName.c_str());

                ImGui::Separator();
                float childHeight = ImGui::GetContentRegionAvail().y - 75.0f; // Leave space for Cancel button
                ImGui::BeginChild("SidebarChild", ImVec2(0, childHeight), false, ImGuiWindowFlags_None);

                //Check for oif we need to do the new fol;der input
                if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    showFoldernameInput = false;
                }
                else if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    pathReference = browsingPath;
                }
                //our current working directory is sandbox/Gamename so go up one
                DrawDirectoryTree(browsingPath, pathReference);

                if (!showFoldernameInput&&ImGui::BeginPopupContextWindow("SidebarContextMenu"))
                {
                    NewFolderPopupThingy(browsingPath);
                    ImGui::EndPopup();
                }
                ImGui::EndChild();
                //end of scrollable section

                ImGui::Separator();
                ImGui::Text("Selected Folder:");
                ImGui::SameLine();


                if (!pathReference.empty())
                {
                    std::filesystem::path relativePath = std::filesystem::relative(pathReference, browsingPath.parent_path());
                    std::string pathStr = relativePath.string()+"\\";

                    size_t copyLen = std::min(pathStr.size(), sizeof(currPathDisplayBuffer) - 1);
                    strncpy(currPathDisplayBuffer, pathStr.c_str(), copyLen);
                    currPathDisplayBuffer[copyLen] = '\0'; 
                }

                //shows our currently selected path, if confirm it closes and performs a check else itll just close
                ImGui::InputText("##SelectedDirForNewFile", currPathDisplayBuffer, sizeof(currPathDisplayBuffer), ImGuiInputTextFlags_ReadOnly);
                if (ImGui::Button("Confirm", ImVec2(120, 0)))
                {
                    if (isNewProj){
                        mSelectedProjPath = pathReference;
                        notInsideAnotherProjFolder = p_ProjMaker.IsNotAnotherGameDirectory(mSelectedProjPath / fixedProjName);
                    }
                    else {
                        ExportSelected(pathReference);
                    }
                    mfileBrowsing = false;

                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    mfileBrowsing = false;
                
                }
  
            
            ImGui::EndPopup();
        }
    }

    /*!*************************************************************************
    This draws the directory tree. Its recursive but only if the directory is open 
    ****************************************************************************/
    void LevelEditor::DrawDirectoryTree(const std::filesystem::path& path, std::filesystem::path& returnPath)
    {
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (!entry.is_directory())
                continue;

            const auto& dirPath = entry.path();
            std::string folderName = dirPath.filename().string();
            bool isSelected = (dirPath == returnPath);

            if (isSelected)
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.5f, 0.8f, 0.8f)); // Blue highlight
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.6f, 0.9f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.5f, 0.7f, 1.0f, 0.8f));
            }

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
            if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

            bool opened = ImGui::TreeNodeEx(folderName.c_str(), flags);

            if (isSelected)
            {
                ImGui::PopStyleColor(3);
            }
            // If clicked, update curr path
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                returnPath = dirPath;
            }

            //this hold priority over the other right click to add a new folder, u  can also delete
            //might need to add a check if u can delete lol
            //actually i should just remove the delete part LOL
            if (ImGui::BeginPopupContextItem(folderName.c_str()))
            {
                showFoldernameInput = true;
                    
                NewFolderPopupThingy(entry.path());

                /*if (ImGui::Selectable("Delete"))
                {
                    std::filesystem::remove_all(entry.path());
                    showFoldernameInput = false;
                }*/

                ImGui::EndPopup();
            }
            

            if (opened)
            {
                DrawDirectoryTree(dirPath,currPath); // recursive call
                ImGui::TreePop();
            }
        }
    }

    /*!*************************************************************************
    This is a little popup function for the file browser if u right click the 
    folder or the explorer u can make a new folder
    ****************************************************************************/
    void LevelEditor::NewFolderPopupThingy(std::filesystem::path newFolderParent) {
        ImGui::Text("New Folder");
        ImGui::SameLine();  // show next to "New Folder"
        ImGui::PushItemWidth(150);  // width of textbox

        //only creates if u press enter
        bool shouldCreate = ImGui::InputText("##newfolder", folderNameBuffer, sizeof(folderNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("OK"))
        {
            shouldCreate = true; // or if u press ok
        }

        //the actual creation code
        if (shouldCreate)
        {
            newFolderName.clear();
            newFolderName = folderNameBuffer;
            // create folder
            if (!newFolderName.empty())
            {
                if (!std::filesystem::exists(newFolderParent / newFolderName)) {
                    try {
                        std::filesystem::create_directory(newFolderParent / newFolderName);
                    }
                    catch (const std::filesystem::filesystem_error& e) {
                        NOCTURNAL_ENGINE_WARN(e.what());
                    }
                }

            }
            showFoldernameInput = false;
        }
    }

    /*!*************************************************************************
    This just notifies the user that they need to close the engine in order to 
    load the project
    ****************************************************************************/
    void LevelEditor::ShowReloadPrompt() {
        ImGui::OpenPopup("Notice");

        // Center the modal
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 125), ImGuiCond_Appearing);

        if (ImGui::BeginPopupModal("Notice", &needCloseToRload))
        {
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1)); // green
            ImGui::TextWrapped("New Game Project files have been generated. Please Exit the editor to reload.");
            ImGui::PopStyleColor(); 

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1)); // red
            ImGui::TextWrapped("Warning: all unsaved files will be lost!");
            ImGui::PopStyleColor();

            ImGui::Spacing();
            if (ImGui::Button("Exit Editor"))
            {
                //closes ther game
                gameShouldClose = true;
                needCloseToRload = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Close"))
            {
                needCloseToRload = false;
            }
            ImGui::EndPopup();
        }
    }
    void LevelEditor::DrawBuildSizeAnalyser() {
        ImGui::OpenPopup("Build Size Analyser");

        // Centers the modal
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(700, 800), ImGuiCond_Appearing);

        if (ImGui::BeginPopupModal("Build Size Analyser", &mbuildSizeAnalyser))
        {
            ImGui::Text("Asset List");
            if (ImGui::Button("Select All"))
            {
                for (std::pair<const std::filesystem::path, bool> &pair :  mbuildCache.GetCache())
                    pair.second = true;

                refreshBuildsize = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Deselect All"))
            {
                for (std::pair<const std::filesystem::path, bool>& pair : mbuildCache.GetCache())
                    pair.second = false;

                refreshBuildsize = true;
            }
            ImGui::Separator();
            ImGui::Spacing();
            float childHeight = ImGui::GetContentRegionAvail().y - 75.0f; // Leave space for Cancel button
            ImGui::BeginChild("AssetListChild", ImVec2(0, childHeight), false, ImGuiWindowFlags_None);

            if (std::filesystem::exists(mAssetsPath))
            {
                AssetListSelectableTree(mAssetsPath);
            }

            ImGui::Text("hi");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::EndChild();
            ImGui::Separator();
            //end of scrollable section
            if (refreshBuildsize)RefreshBuildSize();
            std::string buildSizeTotal = FormatSize(totalBuildBytes);
            ImGui::Text("Total build size: %s", buildSizeTotal.c_str());

            if (ImGui::Button("Export", ImVec2(120, 0)))
            {
                mfileBrowsing = true;
                exportPath.clear();
                exportParentPath = mAssetsPath;
                exportPath = exportParentPath;
            }


            if (mfileBrowsing)
            {
                StupidFileBrowser(exportParentPath, exportPath,false);
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                mbuildSizeAnalyser = false;

            }
            ImGui::EndPopup();
        }
    }
    void LevelEditor::AssetListSelectableTree(const std::filesystem::path& path, bool parentSelectionChanged , bool newSelection ) {
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {

            const auto& dirPath = entry.path();
            std::string folderName = dirPath.filename().string();

            if (mbuildCache.GetCache().find(dirPath)==mbuildCache.GetCache().end())
            {
                mbuildCache.GetCache().emplace(dirPath, true);
            }
            std::string checkboxTitle{ "##" + folderName + "tick" };
            if(parentSelectionChanged)mbuildCache.GetCache()[dirPath] = newSelection;
            bool selectionChanged=ImGui::Checkbox(checkboxTitle.c_str(), &mbuildCache.GetCache()[dirPath]);
            if (parentSelectionChanged)selectionChanged = true;
            
            ImGui::SameLine();
            bool isDir = std::filesystem::is_directory(entry);
            ImGuiTreeNodeFlags flags{};
            if (!isDir) flags |= ImGuiTreeNodeFlags_Leaf;
            else flags|=ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;


            bool opened =ImGui::TreeNodeEx(folderName.c_str(), flags);

            if (!isDir)
            {
                unsigned long long size = std::filesystem::file_size(dirPath);

                std::string sizeStr = FormatSize(size);

                float textWidth = ImGui::CalcTextSize(sizeStr.c_str()).x;
                float availWidth = ImGui::GetContentRegionAvail().x;

                ImGui::SameLine(availWidth - textWidth);
                ImGui::Text("%s", sizeStr.c_str());

            }
            refreshBuildsize |= selectionChanged;


            if (opened)
            {
                if (isDir)
                {
                    AssetListSelectableTree(dirPath,selectionChanged, mbuildCache.GetCache()[dirPath]); // recursive call
                    

                }
                ImVec4 col = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
                col.w *= 0.3f; // reduce alpha for subtle look

                ImGui::PushStyleColor(ImGuiCol_Separator, col);
                ImGui::Separator();
                ImGui::PopStyleColor();
                ImGui::TreePop();
            }

        }
    }

    void LevelEditor::RefreshBuildSize() {
        totalBuildBytes = 0;
        std::vector< std::filesystem::path>toErase{};
        for (const std::pair<std::filesystem::path,bool> pair:mbuildCache.GetCache())
        {
            if (!pair.second)continue;
            if (std::filesystem::exists(pair.first)&&!std::filesystem::is_directory(pair.first))
            {
                totalBuildBytes += std::filesystem::file_size(pair.first);
            }
            if (!std::filesystem::exists(pair.first)) {
                toErase.push_back(pair.first);
            }

        }
        for (std::filesystem::path terase:toErase)
        {
            mbuildCache.GetCache().erase(terase);
        }
        std::filesystem::path exportCachePath{ EngineAssetPath.parent_path() / "ExportSelectionCache.json" };
        mbuildCache.SerializeToFile(exportCachePath.string());
        refreshBuildsize = false;
    }

    void LevelEditor::ExportSelected(const std::filesystem::path& dest)
    {
        std::filesystem::path root = mAssetsPath;
        for (const std::pair<std::filesystem::path, bool> pair : mbuildCache.GetCache())
        {

            if (!pair.second) continue;
            if (!std::filesystem::exists(pair.first))continue;
            if (!std::filesystem::is_regular_file(pair.first)) continue;

            std::filesystem::path relative = std::filesystem::relative(pair.first, root);
            std::filesystem::path target = dest / relative;

            std::error_code ec;

            std::filesystem::create_directories(target.parent_path(), ec);

            std::filesystem::copy_file(
                pair.first,
                target,
                std::filesystem::copy_options::overwrite_existing,
                ec
            );
        }
        mbuildSizeAnalyser = false;
    }
    void LevelEditor::LoadExportCache() {
        std::filesystem::path exportCachePath{ EngineAssetPath.parent_path() / "ExportSelectionCache.json" };
        if (std::filesystem::exists(exportCachePath))
        {
            mbuildCache.DeserializeFromFile(exportCachePath.string());
        }

    }
    std::string LevelEditor::FormatSize(std::uintmax_t size)
    {
        const char* suffix[] = { "B", "KB", "MB", "GB" };
        int i = 0;
        double dblSize = size;

        while (dblSize >= 1024 && i < 3)
        {
            dblSize /= 1024;
            i++;
        }

        char buf[64];
        sprintf(buf, "%.2f %s", dblSize, suffix[i]);
        return buf;
    }
}
