/*!------------------------------------------------------------------------------------------------
    File: LevelEditor.cpp
    Project: NocturnalStudio

	Brief:
        The level editor utilises Dear ImGui and OpenGL to create a editor interface
        to allow us to edit object, create entities and modify the
        properties of the components and save them accordingly.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the LevelEditor class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>

#include "LevelEditor/LevelEditor.h"
#include "Timer/Time.h"
#include "Timer/Fps.h"
#include "Graphics/Renderer.h"
#include "Graphics/Graphics.h"
#include "LevelEditor/Picker.h"

#include "Graphics/FrameBuffer.h"
#include "Core/Log.h"
#include "Audio/AudioEngine.h"

#include "ECS/Components/Components.h"
#include "ECS/SceneManager.h"
#include "Core/MemoryManager.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Scripts/ButtonResponse.h"
#include "Scripts/ScenerioScript.h"
#include "Scripts/AudioManager.h"

#include "Scripts/BackgroundAudio.h"
#include "Scripts/SliderScript.h"
#include "Scripts/PauseMenu.h"
#include "Scripts/CutScene.h"
#include "Scripts/DialogueManager.h"
#include "Scripts/DragAndDrop.h"
#include "Scripts/HealthBar.h"
#include "Scripts/AnimalLogic.h"
#include "Scripts/TimerDisplay.h"

#include "Scripts/GlobalGameManager.h"


#include <unordered_map>
#include <array>
#include <vector>
#include <algorithm>


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

        LoadSceneFromFile();

        LoadScriptsFromFile();

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
        MainMenuBar();

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
#if DEBUG
        // std::string scriptPath = "../ExoEngine/src/ExoEngine/Scripts";
        std::string scriptPath = "../../Engine/include/Scripts";
#else
        std::string scriptPath = "Assets/Scripts"; //for release mode
#endif
        for (auto const& dir_entry : std::filesystem::directory_iterator{ scriptPath })
        {
            if (!dir_entry.is_regular_file())
            {
                continue;
            }
            if (*(dir_entry.path().filename().string().end() - 1) == 'h')
            {
                std::string in = dir_entry.path().filename().string();
                in.pop_back();
                in.pop_back();
                mScriptList.emplace_back(in);
            }
        }
    }

    /*!*************************************************************************
    Menu bar located in the top left side of the window is used to toggle between
    opening and closing the editor
    ****************************************************************************/
    void LevelEditor::MainMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (is_ShowWindow)
            {
                if (ImGui::Button("Close Editor/Play"))
                {
				    is_ShowWindow = false;
                    
                    std::vector<std::string> playablescenes = {
                       "FlamingoScene.json",
                       "HorseScene.json",
                       "SealScene.json",
                       "OwlScene.json" };
                    for (std::string s : playablescenes)
                    {
                        if (s == mCurrentScene) { p_Scene->isGameplay = true; break; }
                    }
                    p_GameManager.SetState(GameManager::GameState::STARTING);
                }
            }
            else
            {
                if (ImGui::Button("Open Editor/Stop"))
                {
                    is_ShowWindow = true;
                    p_Scene->isGameplay = false;
                    p_GameManager.ResetSystem(true);
                   
                    
                    
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
        //ImGui::SameLine();

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
        ImGuizmo::BeginFrame();
        if (selectedEntity >= 0 && selectedEntity < MAX_ENTITIES && isTransformOpen&&p_ecs.HaveComponent<Transform>(selectedEntity))//have selected entity
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
        }

        ImGui::End();
        ImGui::PopStyleVar();
        std::multimap<float, Transform*> sortedMultimap;
        for (Entity entity = 0; entity < p_ecs.GetTotalEntities(); entity++)
        {
            if(p_ecs.HaveComponent<Transform>(entity))
                sortedMultimap.insert({ p_ecs.GetComponent<Transform>(entity).GetRot(),
                            &p_ecs.GetComponent<Transform>(entity) });
        }
        if (p_ecs.GetTotalEntities() != 0 && p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT) && !ImGuizmo::IsOver())
        {
            if (selectedEntity >= 0 && selectedEntity < MAX_ENTITIES)//no entity selected will remain to the previous selected entity
            {
                if(p_ecs.HaveComponent<Transform>(selectedEntity))
                    selectedEntity = (Entity)Picker::Pick(NocturnalEngine::Graphic::mcamera, sortedMultimap);
            }
            else
                selectedEntity = 0;
          
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
        {
            if (selectedEntity >= 0 && selectedEntity < MAX_ENTITIES)
            {
                p_ecs.DestroyEntity(selectedEntity);
                selectedEntity = {};
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
                if (ext == ".png")
                {
                    std::string texName = filename;
                    texName.erase(filename.find_last_of('.'));

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
                            std::string str = texName + " " + "(" + std::to_string(tex->GetWidth()) + "x" +
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
                    ImGui::ImageButton("SceneButton",(ImTextureID)(intptr_t)ResourceManager::GetIcon("TextIcon")->GetRendererID(),
                        ImVec2(thumbnailSize, thumbnailSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        std::string jsnName = filename;
                        jsnName.erase(filename.find_last_of('.'));

                        std::string str{};

                        if (mCurrentDirectory.filename() == "Scene") {
                            str += "Scene: ";
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

                            p_Scene->DeserializeFromFile(mCurrentDirectory.string()+'/' + filename);
                            mCurrentScene = filename;

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
                        ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", payloadStr.c_str(), payloadStr.size() + 1);

                        ImGui::Image((ImTextureID)(intptr_t)ResourceManager::GetIcon("SoundIcon")->GetRendererID(),
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
    void LevelEditor::Logger()
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

            //ImGui::Separator();
            //ImGui::Text("Layering System (Only shows used layers)");
            //ImGui::Text("Top draws first (behind). Bottom draws last (in front).");
            //ImGui::Separator();

            //// 1) Collect which layers are actually used by sprites in the scene
            //std::vector<int> usedLayers;
            //usedLayers.reserve(NocturnalEngine::Graphic::MAX_LAYERS);

            //for (Entity e = 0; e < p_ecs.GetTotalEntities(); ++e)
            //{
            //    if (!p_ecs.HaveComponent<Sprite>(e)) continue;

            //    int l = p_ecs.GetComponent<Sprite>(e).LayerOrder;
            //    if (l < 0) l = 0;
            //    if (l >= NocturnalEngine::Graphic::MAX_LAYERS) l = NocturnalEngine::Graphic::MAX_LAYERS - 1;

            //    usedLayers.push_back(l);
            //}

            //// If no sprites exist yet, at least show layer 0
            //if (usedLayers.empty())
            //    usedLayers.push_back(0);

            //// Unique + sort
            //std::sort(usedLayers.begin(), usedLayers.end());
            //usedLayers.erase(std::unique(usedLayers.begin(), usedLayers.end()), usedLayers.end());

            //// 2) Build the visible list in CURRENT draw order (so UI reflects current draw order)
            //std::vector<int> visibleLayers;
            //visibleLayers.reserve(usedLayers.size());

            //for (int i = 0; i < NocturnalEngine::Graphic::MAX_LAYERS; ++i)
            //{
            //    int layer = NocturnalEngine::Graphic::layerDrawOrder[i];

            //    if (std::binary_search(usedLayers.begin(), usedLayers.end(), layer))
            //        visibleLayers.push_back(layer);
            //}

            //// 3) UI: drag to reorder ONLY visible layers,
            //// but we update the REAL global array: Graphic::layerDrawOrder[]
            //for (int visIdx = 0; visIdx < (int)visibleLayers.size(); ++visIdx)
            //{
            //    int layer = visibleLayers[visIdx];
            //    ImGui::PushID(visIdx);

            //    ImGui::Checkbox("##enable", &NocturnalEngine::Graphic::layerEnabled[layer]);
            //    ImGui::SameLine();

            //    std::string label = "Layer " + std::to_string(layer);
            //    ImGui::Selectable(label.c_str(), false);

            //    // Drag source: drag by visible index
            //    if (ImGui::BeginDragDropSource())
            //    {
            //        ImGui::SetDragDropPayload("LAYER_REORDER_VISIBLE", &visIdx, sizeof(int));
            //        ImGui::Text("Move %s", label.c_str());
            //        ImGui::EndDragDropSource();
            //    }

            //    // Drop target
            //    if (ImGui::BeginDragDropTarget())
            //    {
            //        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LAYER_REORDER_VISIBLE"))
            //        {
            //            int srcVisIdx = *(const int*)payload->Data;
            //            int dstVisIdx = visIdx;

            //            if (srcVisIdx != dstVisIdx &&
            //                srcVisIdx >= 0 && srcVisIdx < (int)visibleLayers.size() &&
            //                dstVisIdx >= 0 && dstVisIdx < (int)visibleLayers.size())
            //            {
            //                int movingLayer = visibleLayers[srcVisIdx];
            //                int dstLayer = visibleLayers[dstVisIdx];

            //                // Find their positions in the GLOBAL drawOrder array
            //                int srcGlobal = -1, dstGlobal = -1;
            //                for (int g = 0; g < NocturnalEngine::Graphic::MAX_LAYERS; ++g)
            //                {
            //                    if (NocturnalEngine::Graphic::layerDrawOrder[g] == movingLayer) srcGlobal = g;
            //                    if (NocturnalEngine::Graphic::layerDrawOrder[g] == dstLayer)    dstGlobal = g;
            //                }

            //                if (srcGlobal != -1 && dstGlobal != -1)
            //                {
            //                    // Stable "move" in global draw order
            //                    int temp = NocturnalEngine::Graphic::layerDrawOrder[srcGlobal];

            //                    if (srcGlobal < dstGlobal)
            //                    {
            //                        for (int k = srcGlobal; k < dstGlobal; ++k)
            //                            NocturnalEngine::Graphic::layerDrawOrder[k] = NocturnalEngine::Graphic::layerDrawOrder[k + 1];
            //                        NocturnalEngine::Graphic::layerDrawOrder[dstGlobal] = temp;
            //                    }
            //                    else
            //                    {
            //                        for (int k = srcGlobal; k > dstGlobal; --k)
            //                            NocturnalEngine::Graphic::layerDrawOrder[k] = NocturnalEngine::Graphic::layerDrawOrder[k - 1];
            //                        NocturnalEngine::Graphic::layerDrawOrder[dstGlobal] = temp;
            //                    }
            //                }
            //            }
            //        }
            //        ImGui::EndDragDropTarget();
            //    }

            //    ImGui::PopID();
            //}



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



    void LevelEditor::Layering()
    {
        if (!is_ShowWindow) return;

        ImGui::Begin("Layering");

        ImGui::Text("Layering System (Only shows used layers)");
        ImGui::Text("Top draws first (behind). Bottom draws last (in front).");
        ImGui::Separator();

        std::vector<int> usedLayers;
        usedLayers.reserve(NocturnalEngine::Graphic::MAX_LAYERS);

        for (Entity e = 0; e < p_ecs.GetTotalEntities(); ++e)
        {
            if (!p_ecs.HaveComponent<Sprite>(e)) continue;

            int l = p_ecs.GetComponent<Sprite>(e).LayerOrder;
            if (l < 0) l = 0;
            if (l >= NocturnalEngine::Graphic::MAX_LAYERS) l = NocturnalEngine::Graphic::MAX_LAYERS - 1;

            usedLayers.push_back(l);
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
            if (ImGui::Button("Create Entity") && p_ecs.GetTotalEntities() != MAX_ENTITIES)
            {
                p_ecs.AddComponent<NameTag>(p_ecs.CreateEntity(), C_NameTagComponent);
            }

            if (p_ecs.GetTotalEntities() > 0)
            {
                ImGui::SameLine();
                if (ImGui::Button("Destroy Entity"))
                {
                    if (selectedEntity != MAX_ENTITIES)
                    {
                        p_ecs.DestroyEntity(selectedEntity);
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Clone Entity") && p_ecs.GetTotalEntities() != 0)// there is entity alive
                {
                    if (selectedEntity != MAX_ENTITIES)
                    {
                        Entity CloneEntity = p_ecs.CloneEntity(selectedEntity);
                        selectedEntity = CloneEntity; // when the entity is destroy there is no current selected entity
                    }
                }

                auto aliveTotal = p_ecs.GetTotalEntities();
                
                Entity aliveCount = 0;
                Entity iterEntity = 0;
                while (aliveCount < aliveTotal)
                {
                    if (p_ecs.HaveComponent<NameTag>(iterEntity))
                    {
                        
                        const auto& tag = p_ecs.GetComponent<NameTag>(iterEntity).GetNameTag();

                        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow;
                        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)iterEntity, flags, tag.c_str());

                        if (ImGui::IsItemClicked())
                            selectedEntity = iterEntity;

                        if (opened)
                            ImGui::TreePop();

                        aliveCount++;
                    }
                    else {
                        NameTag nt;
                        nt.SetComponentEntityID(iterEntity);
                        nt.SetNameTag("Entity " + std::to_string(iterEntity));
                        p_ecs.AddComponent<NameTag>(iterEntity, nt);
                        aliveCount++;
                    }

                    iterEntity++;
                }
            }

            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
                selectedEntity = {};
            ImGui::End();
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
            if (selectedEntity < MAX_ENTITIES && selectedEntity >= 0 && p_ecs.GetTotalEntities() != 0)// if the selectedEntityExist
            {
                //create component for the selected entity 
                if (ImGui::Button("Add Component"))
                    ImGui::OpenPopup("Add Component");

                if (ImGui::BeginPopup("Add Component"))
                {

                    if (ImGui::MenuItem("Transform"))
                    {
                        if (!p_ecs.HaveComponent<Transform>(selectedEntity))
                            p_ecs.AddComponent<Transform>(selectedEntity, C_TransformComponent);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Sprite"))
                    {
                        if (!p_ecs.HaveComponent<Sprite>(selectedEntity))
                            p_ecs.AddComponent<Sprite>(selectedEntity, C_SpriteComponent);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Collider"))
                    {
                        if (!p_ecs.HaveComponent<Collider>(selectedEntity))
                            p_ecs.AddComponent<Collider>(selectedEntity, C_ColliderComponent);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("RigidBody"))
                    {
                        if (!p_ecs.HaveComponent<RigidBody>(selectedEntity))
                            p_ecs.AddComponent<RigidBody>(selectedEntity, C_RigidBodyComponent);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Logic"))
                    {
                        if (!p_ecs.HaveComponent<Logic>(selectedEntity))
                            p_ecs.AddComponent<Logic>(selectedEntity, C_LogicComponent);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Audio"))
                    {
                        if (!p_ecs.HaveComponent<Audio>(selectedEntity))
                            p_ecs.AddComponent<Audio>(selectedEntity, C_AudioComponent);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Tag"))
                    {
                        if (!p_ecs.HaveComponent<Tag>(selectedEntity))
                            p_ecs.AddComponent<Tag>(selectedEntity, C_TagComponent);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("HUD"))
                    {
                        if (!p_ecs.HaveComponent<HUDComponent>(selectedEntity))
                            p_ecs.AddComponent<HUDComponent>(selectedEntity, C_HUDComponent);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Text"))
                    {
                        if (!p_ecs.HaveComponent<Text>(selectedEntity))
                            p_ecs.AddComponent<Text>(selectedEntity, C_TextComponent);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Animal Attributes"))
                    {
                        if (!p_ecs.HaveComponent<AnimalAttributes>(selectedEntity))
                            p_ecs.AddComponent<AnimalAttributes>(selectedEntity, C_AnimalAttributesComponent);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Items Attributes"))
                    {
                        if (!p_ecs.HaveComponent<ItemsAttributes>(selectedEntity))
                            p_ecs.AddComponent<ItemsAttributes>(selectedEntity, C_ItemsAttributesComponent);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Draggable"))
                    {
                        if (!p_ecs.HaveComponent<Draggable>(selectedEntity))
                            p_ecs.AddComponent<Draggable>(selectedEntity, C_DraggableComponent);
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
                //Check and change the name of the Entity
                if (p_ecs.HaveComponent<NameTag>(selectedEntity))
                {
                    if (ImGui::CollapsingHeader("Name", ImGuiTreeNodeFlags_None))
                    {
                        auto& name = p_ecs.GetComponent<NameTag>(selectedEntity).GetNameTag();
                        char buffer[256];
                        memset(buffer, 0, sizeof(buffer));
                        strcpy_s(buffer, sizeof(buffer), name.c_str());
                        if (ImGui::InputText("name", buffer, sizeof(buffer)))
                        {
                            name = std::string(buffer);
                        }
                    }
                }
                //If Entity have Transform Component
                if (p_ecs.HaveComponent<Transform>(selectedEntity))
                {
                    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_None))
                    {
                        if (!isTransformOpen)isTransformOpen = true;
                        //position
                        auto& Position = p_ecs.GetComponent<Transform>(selectedEntity).GetPos();
                        ImGui::PushItemWidth(100.0f);
                        ImGui::Text("Position"); ImGui::SameLine();
                        ImGui::Text("X"); ImGui::SameLine();                    //set a "x" to indicate x-axis
                        ImGui::DragFloat("##PositionX", (float*)&Position.x, 0.005f); ImGui::SameLine(); //char name , pass float pointer to position vec2D which hold x and y, the scaling value in imgui
                        ImGui::PushID(1);
                        ImGui::Text("Y"); ImGui::SameLine();
                        ImGui::DragFloat("##PositionY", (float*)&Position.y, 0.005f);
                        ImGui::PopID();

                        //scale
                        auto& Scale = p_ecs.GetComponent<Transform>(selectedEntity).GetScale();
                        ImGui::Text("Scale   "); ImGui::SameLine();
                        ImGui::Text("X"); ImGui::SameLine();                    //set a "x" to indicate x-axis
                        ImGui::DragFloat("##Scale", (float*)&Scale.x, 0.005f); ImGui::SameLine(); //char name , pass float pointer to position vec2D which hold x and y, the scaling value in imgui
                        ImGui::PushID(2);
                        ImGui::Text("Y"); ImGui::SameLine();
                        ImGui::DragFloat("##Scale", (float*)&Scale.y, 0.005f);
                        ImGui::PopID();

                        //rotation
                        auto& rotation = p_ecs.GetComponent<Transform>(selectedEntity).GetRot();
                        ImGui::Text("Rotation Z"); ImGui::SameLine();
                        ImGui::DragFloat("##", (float*)&rotation, 1.0f);
                    }
                    else if (isTransformOpen) {
                        isTransformOpen = false;
                    }
                }
                //Sprite Component
                if (p_ecs.HaveComponent<Sprite>(selectedEntity))
                {

                    if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_None))
                    {
                        auto& sprite = p_ecs.GetComponent<Sprite>(selectedEntity);
                        ImGui::Checkbox("SpriteSheet", &sprite.is_SpriteSheet); ImGui::SameLine();
                        ImGui::Checkbox("Animation", &sprite.is_Animated);
                        ImGui::PushItemWidth(100.0f);
                        ImGui::Text("Coordinates: "); ImGui::SameLine();
                        ImGui::Text("X"); ImGui::SameLine();
                        ImGui::DragInt("##X", (int*)&sprite.GetIndex().x, 1); ImGui::SameLine();
                        ImGui::Text("Y"); ImGui::SameLine();
                        ImGui::DragInt("##Y", (int*)&sprite.GetIndex().y, 1);
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4());
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4());
                        ImGui::Text("Image : "); ImGui::SameLine();
                        ImGui::PopStyleColor(3);
                        auto& texturePath = p_ecs.GetComponent<Sprite>(selectedEntity).GetTexture();
                        ImGui::SetNextItemWidth(140.0f);

                        if (ImGui::BeginCombo("##sprite", texturePath.c_str()))
                        {
                            for (auto& [str, tex] : ResourceManager::textures)
                            {
                                if (ImGui::Selectable(str.c_str()))
                                {
                                    texturePath = str;
                                    NOCTURNAL_ENGINE_INFO("Loaded {0} Sprite", texturePath.c_str());
                                }
                            }
                            ImGui::EndCombo();
                        }

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Textures"))
                            {
                                texturePath = *(const std::string*)payload->Data;
                            }
                            ImGui::EndDragDropTarget();
                        }
                        if (sprite.is_SpriteSheet)
                        {
                            sprite.GetMaxIndex() = (int)GETTEXTURE(sprite.GetTexture())->GetWidth() / 512;
                            ImGui::Text("MaxIndex : %d ", sprite.GetMaxIndex());
                            sprite.GetDisplayTime().resize(sprite.GetMaxIndex());//resize the number of frames in a sprite  
                        }
                        if (sprite.GetMaxIndex() > 0 && sprite.is_Animated)
                        {
                            ImGui::Text("SelectedIndex: "); ImGui::SameLine();
                            ImGui::DragInt("##SelectedIndex", (int*)&sprite.GetIndex().x, 1, 0, sprite.GetMaxIndex() - 1);

                            ImGui::Text("SetDisplayTime: "); ImGui::SameLine();
                            ImGui::DragFloat("##SetDisplayTime", (float*)&sprite.GetDisplayTime()[sprite.GetIndex().x], 0.005f, 0.0f);
                            
                        }    
                        
                        if (sprite.is_SpriteSheet)
                        {
                            ImGui::Text("UVCoordinates: "); ImGui::SameLine();
                            ImGui::Text("U"); ImGui::SameLine();
                            ImGui::DragFloat("##U", (float*)&sprite.GetUVCoor().x, 0.5f); ImGui::SameLine();
                            ImGui::PushID(2);
                            ImGui::Text("V"); ImGui::SameLine();
                            ImGui::DragFloat("##V", (float*)&sprite.GetUVCoor().y, 0.5f);
                            ImGui::PopID();
                        }
                        ImGui::Text("Layering Order: "); ImGui::SameLine();
                        // ImGui::DragInt("##layering", (int*)&sprite.LayerOrder, 1, 0, 8);
                        ImGui::DragInt("##layering", (int*)&sprite.LayerOrder, 1, 0, NocturnalEngine::Graphic::MAX_LAYERS - 1);
                    }
                }
                //Text Component
                if (p_ecs.HaveComponent<Text>(selectedEntity))
                {
                    if (ImGui::CollapsingHeader("Text", ImGuiTreeNodeFlags_None))
                    {
                        auto& text = p_ecs.GetComponent<Text>(selectedEntity);

                        char buffer[1024];
                        memset(buffer, 0, sizeof(buffer));
                        strcpy_s(buffer, sizeof(buffer), text.GetContent().c_str());
                        if (ImGui::InputTextMultiline("Content", buffer, sizeof(buffer), ImVec2(0, 80)))
                        {
                            text.SetContent(std::string(buffer));
                        }

                        auto& fontName = text.GetFont();
                        ImGui::Text("Font"); ImGui::SameLine();
                        ImGui::SetNextItemWidth(180.0f);
                        if (ImGui::BeginCombo("##textfont", fontName.c_str()))
                        {
                            for (auto& [name, font] : ResourceManager::fonts)
                            {
                                bool selected = (name == fontName);
                                if (ImGui::Selectable(name.c_str(), selected))
                                {
                                    fontName = name;
                                }
                                if (selected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }

                        ImGui::DragFloat("Size", &text.GetSize(), 0.1f, 1.0f, 200.0f);
                        ImGui::ColorEdit4("Color", text.GetColor());
                        ImGui::Checkbox("World Space", &text.GetWorldSpace());
                        ImGui::Checkbox("Center", &text.GetCenter());

                        ImGui::Text("Position");
                        ImGui::Text("X"); ImGui::SameLine();
                        ImGui::DragFloat("##TextPosX", &text.GetPosition()[0], 0.01f);
                        ImGui::Text("Y"); ImGui::SameLine();
                        ImGui::DragFloat("##TextPosY", &text.GetPosition()[1], 0.01f);
                        ImGui::Text("Line Height"); ImGui::SameLine();
                        ImGui::DragInt("##Line Height", &text.Get_line_height(), 1);
                    }
                }
                //Collider Component
                if (p_ecs.HaveComponent<Collider>(selectedEntity))
                {
                    if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_None))
                    {
                        //selection for collider type
                        auto& colliderComp = p_ecs.GetComponent<Collider>(selectedEntity);
                        auto& colliderType = colliderComp[0].mCol;
                        int colliderIndex = static_cast<int>(colliderType);
                        const char* colliderNames = "none\0circle\0bubble\0line\0rect\0box\0bossball\0button";
                        ImGui::Text("Collider 1 Type"); ImGui::SameLine();
                        ImGui::Combo("##test", &colliderIndex, colliderNames);
                        colliderType = static_cast<Collider::ColliderType>(colliderIndex);

                        //positioning the offset
                        auto& colliderOffset = colliderComp[0].mOffset;
                        ImGui::PushItemWidth(100.0f);

                        ImGui::Text("OffSet   "); ImGui::SameLine();
                        ImGui::Text("X"); ImGui::SameLine();

                        ImGui::DragFloat("##colliderOffsetX", (float*)&colliderOffset.x, 0.05f); ImGui::SameLine();
                        ImGui::PushID(1);
                        ImGui::Text("Y"); ImGui::SameLine();
                        ImGui::DragFloat("##colliderOffsetY", (float*)&colliderOffset.y, 0.05f);
                        ImGui::PopID();

                         //size of the collider
                        if (colliderComp[0].mCol == Collider::ColliderType::circle || colliderComp[0].mCol == Collider::ColliderType::bubble ||
                            colliderComp[0].mCol == Collider::ColliderType::bossball)
                        {
                            auto& colliderSize = colliderComp[0].mRadius;
                            ImGui::Text("Radius   "); ImGui::SameLine();
                            ImGui::DragFloat("##Radius", (float*)&colliderSize, 0.05f);
                        }
                        else if (colliderComp[0].mCol == Collider::ColliderType::rect || colliderComp[0].mCol == Collider::ColliderType::box)
                        {
                            ImGui::Text("Minimum "); ImGui::SameLine();
                            ImGui::Text("X"); ImGui::SameLine();
                            ImGui::DragFloat("##MinimumX", (float*)&colliderComp[0].mMin.x, 0.05f); ImGui::SameLine();
                            ImGui::Text("Y"); ImGui::SameLine();
                            ImGui::DragFloat("##MinimumY", (float*)&colliderComp[0].mMin.y, 0.05f);

                            ImGui::Text("Maximum "); ImGui::SameLine();
                            ImGui::Text("X"); ImGui::SameLine();
                            ImGui::DragFloat("##MaximumX", (float*)&colliderComp[0].mMax.x, 0.05f); ImGui::SameLine();
                            ImGui::Text("Y"); ImGui::SameLine();
                            ImGui::DragFloat("##MaximumY", (float*)&colliderComp[0].mMax.y, 0.05f);
                        }

                        //selection for collider 2 type
                        auto& colliderType1 = colliderComp[1].mCol;
                        int colliderIndex1 = static_cast<int>(colliderType1);
                        const char* colliderNames1 = "none\0circle\0bubble\0line\0rect\0box\0button";
                        ImGui::Text("Collider 2 Type"); ImGui::SameLine();
                        ImGui::Combo("###test", &colliderIndex1, colliderNames1);
                        ImGui::PushItemWidth(100.0f);
                        colliderType1 = static_cast<Collider::ColliderType>(colliderIndex1);

                        //positioning the offset
                        auto& colliderOffset1 = colliderComp[1].mOffset;

                        ImGui::Text("OffSet   "); ImGui::SameLine();
                        ImGui::Text("X"); ImGui::SameLine();

                        ImGui::DragFloat("###colliderOffsetX", (float*)&colliderOffset1.x, 0.05f); ImGui::SameLine();
                        ImGui::PushID(1);
                        ImGui::Text("Y"); ImGui::SameLine();
                        ImGui::DragFloat("###colliderOffsetY", (float*)&colliderOffset1.y, 0.05f);
                        ImGui::PopID();

                         //size of the collider
                        if (colliderComp[1].mCol == Collider::ColliderType::circle || colliderComp[1].mCol == Collider::ColliderType::bubble ||
                            colliderComp[1].mCol == Collider::ColliderType::bossball)
                        {
                            auto& colliderSize1 = colliderComp[1].mRadius;
                            ImGui::Text("Radius   "); ImGui::SameLine();
                            ImGui::DragFloat("###Radius", (float*)&colliderSize1, 0.05f);
                        }
                        else if (colliderComp[1].mCol == Collider::ColliderType::rect || colliderComp[1].mCol == Collider::ColliderType::box)
                        {
                            ImGui::Text("Minimum "); ImGui::SameLine();
                            ImGui::Text("X"); ImGui::SameLine();
                            ImGui::DragFloat("###MinimumX", (float*)&colliderComp[1].mMin.x, 0.05f); ImGui::SameLine();
                            ImGui::Text("Y"); ImGui::SameLine();
                            ImGui::DragFloat("###MinimumY", (float*)&colliderComp[1].mMin.y, 0.05f);

                            ImGui::Text("Maximum "); ImGui::SameLine();
                            ImGui::Text("X"); ImGui::SameLine();
                            ImGui::DragFloat("###MaximumX", (float*)&colliderComp[1].mMax.x, 0.05f); ImGui::SameLine();
                            ImGui::Text("Y"); ImGui::SameLine();
                            ImGui::DragFloat("###MaximumY", (float*)&colliderComp[1].mMax.y, 0.05f);
                        }
                    }
                }

                //Logic
                if (p_ecs.HaveComponent<Logic>(selectedEntity))
                {
                    if (ImGui::CollapsingHeader("Logic", ImGuiTreeNodeFlags_None))
                    {
                        auto& logic = p_ecs.GetComponent<Logic>(selectedEntity);
                        
                        static ImGuiComboFlags flags = 0;
                        static int current_script = 0;
                        const size_t aSize = 30;
                        const char* logicList[aSize];
                        std::vector<const char*> sList;

                        for (int i = 0; i < mScriptList.size(); i++)
                        {
                            sList.push_back(mScriptList[i].c_str()); 
                        }
                        std::copy(sList.begin(), sList.end(), logicList);

                        ImGui::Combo("Logic Scripts", &current_script, logicList, static_cast<int>(sList.size()), static_cast<int>(sList.size()));

                        if (ImGui::Button("Add"))
                        {
                            bool can_addScript = true; 
                            for (int i = 0; i < logic.GetScriptNames().size(); i++) 
                            {
                                if (logic.GetScriptNames()[i] == mScriptList[current_script]) 
                                {
                                    can_addScript = false;
                                }
                            }
                            if (can_addScript == true)
                            {
                                if (p_ecs.HaveComponent<NameTag>(selectedEntity) && p_ecs.HaveComponent<Transform>(selectedEntity))
                                {
                                    if (mScriptList[current_script] == "ButtonResponse" && p_ecs.HaveComponent<Collider>(selectedEntity) && p_ecs.HaveComponent<Tag>(selectedEntity)
                                                                                        && p_ecs.HaveComponent<Sprite>(selectedEntity))
                                    {
                                        logic.InsertScript(new ButtonResponse(), selectedEntity);
                                    }

                                    if (mScriptList[current_script] == "PauseMenu" && p_ecs.HaveComponent<Collider>(selectedEntity) && p_ecs.HaveComponent<Tag>(selectedEntity)
                                                                                   && p_ecs.HaveComponent<Sprite>(selectedEntity))
                                    {
                                        logic.InsertScript(new PauseMenu(), selectedEntity);
                                    }
                                    if (mScriptList[current_script] == "ScenerioScript")
                                    {
                                        logic.InsertScript(new ScenerioScript(), selectedEntity);
                                    }
                                    if (mScriptList[current_script] == "AudioManager" && p_ecs.HaveComponent<Audio>(selectedEntity))
                                    {
                                        logic.InsertScript(new AudioManager(), selectedEntity);
                                    }
                                    if (mScriptList[current_script] == "BackgroundAudio" && p_ecs.HaveComponent<Audio>(selectedEntity))
                                    {
                                        logic.InsertScript(new BackgroundAudio(), selectedEntity);
                                    }
                                    if (mScriptList[current_script] == "SliderScript" && p_ecs.HaveComponent<Collider>(selectedEntity) && p_ecs.HaveComponent<Tag>(selectedEntity)
                                                                                      && p_ecs.HaveComponent<Sprite>(selectedEntity) && p_ecs.HaveComponent<NameTag>(selectedEntity))
                                    {
                                        logic.InsertScript(new SliderScript(), selectedEntity);
                                    }
                                    if (mScriptList[current_script] == "CutScene" && p_ecs.HaveComponent<Sprite>(selectedEntity) && p_ecs.HaveComponent<NameTag>(selectedEntity))
                                    {
                                        logic.InsertScript(new CutScene(), selectedEntity);
                                    }
                                    if (mScriptList[current_script] == "DialogueManager" && p_ecs.HaveComponent<Sprite>(selectedEntity))
                                    {
                                        logic.InsertScript(new DialogueManager(), selectedEntity);
                                    }
                                    if (mScriptList[current_script] == "DragAndDrop" && p_ecs.HaveComponent<Sprite>(selectedEntity)&& p_ecs.HaveComponent<ItemsAttributes>(selectedEntity))
                                    {
                                        logic.InsertScript(new DragAndDrop(), selectedEntity);
                                    }
                                    if (mScriptList[current_script] == "HealthBar" && p_ecs.HaveComponent<Sprite>(selectedEntity)&& p_ecs.HaveComponent<HUDComponent>(selectedEntity))
                                    {
                                        logic.InsertScript(new HealthBar(), selectedEntity);
                                    }
                                    if (mScriptList[current_script] == "AnimalLogic" && p_ecs.HaveComponent<AnimalAttributes>(selectedEntity))
                                    {
                                        logic.InsertScript(new AnimalLogic(), selectedEntity);
                                    }
                                    if (mScriptList[current_script] == "TimerDisplay" && p_ecs.HaveComponent<Text>(selectedEntity))
                                    {
                                        logic.InsertScript(new TimerDisplay(), selectedEntity);
                                    }
                                }
                            }
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Delete")) {
                            logic.DeleteScript(mScriptList[current_script]);
                        }
                        ImGui::Text("Inserted Scripts:");
                        for (int i = 0; i < logic.GetScriptNames().size(); i++) {
                            //display the names of all scripts in the entity
                            ImGui::Text("% s", logic.GetScriptNames()[i].c_str());
                        }
                    }
                }
                //Rigid Component
                if (p_ecs.HaveComponent<RigidBody>(selectedEntity))
                {
                    if (ImGui::CollapsingHeader("RigidBody", ImGuiTreeNodeFlags_None))
                    {
                        //Acceleration
                        auto& velocity = p_ecs.GetComponent<RigidBody>(selectedEntity).GetAccel();
                        ImGui::PushItemWidth(100.0f);
                        ImGui::Text("Acceleration   "); ImGui::SameLine();
                        ImGui::Text("X"); ImGui::SameLine();
                        ImGui::DragFloat("##AccelerationX", (float*)&velocity.x, 0.005f); ImGui::SameLine();
                        ImGui::PushID(3);
                        ImGui::Text("Y"); ImGui::SameLine();
                        ImGui::DragFloat("##AccelerationY", (float*)&velocity.y, 0.005f);
                        ImGui::PopID();

                        //Direction
                        auto& direction = p_ecs.GetComponent<RigidBody>(selectedEntity).GetDir();
                        ImGui::PushItemWidth(100.0f);
                        ImGui::Text("Direction  "); ImGui::SameLine();
                        ImGui::Text("X"); ImGui::SameLine();
                        ImGui::DragFloat("##DirectionX", (float*)&direction.x, 0.005f); ImGui::SameLine();
                        ImGui::PushID(4);
                        ImGui::Text("Y"); ImGui::SameLine();
                        ImGui::DragFloat("##DirectionY", (float*)&direction.y, 0.005f);
                        ImGui::PopID();

                        //GetFricition
                        auto& friction = p_ecs.GetComponent<RigidBody>(selectedEntity).GetFriction();
                        ImGui::Text("Friction   "); ImGui::SameLine();
                        ImGui::DragFloat("##Friction", (float*)&friction, 1.0f);

                        //GetRestitution
                        auto& Restitution = p_ecs.GetComponent<RigidBody>(selectedEntity).GetRestitution();
                        ImGui::Text("Restitution"); ImGui::SameLine();
                        ImGui::DragFloat("##Restitution", (float*)&Restitution, 1.0f);
                    }
                }
                //Audio Component
                if (p_ecs.HaveComponent<Audio>(selectedEntity))
                {
                    if (ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_None))
                    {
                        auto& AudioComp = p_ecs.GetComponent<Audio>(selectedEntity);
                        auto& AudioArr = AudioComp.GetArr();
                        //list out all audio files loaded
                        static int currentfile = 0;
                        const size_t arraysize = 30;
                        const char* audioFileList[arraysize];
                        std::vector<const char*> audiofilenames;
                        for (auto& item : mAudioFile) {
                            audiofilenames.push_back(item.c_str());
                        }
                        std::copy(audiofilenames.begin(), audiofilenames.end(), audioFileList);
                        ImGui::Text("Loaded Audio Files:");
                        ImGui::Combo("##Audio Files", &currentfile, audioFileList, static_cast<int>(audiofilenames.size()), static_cast<int>(audiofilenames.size()));
                        //list out audio file paths in component
                        static ImGuiComboFlags flags = 0;
                        static int current_audio = 0;
                        const size_t aSize = 30;
                        const char* AudioList[aSize];
                        std::vector<const char*> sList;
                        for (int i = 0; i < AudioArr.size(); i++) {
                            sList.push_back(AudioArr[i].mAudioPath.c_str());
                        }
                        std::copy(sList.begin(), sList.end(), AudioList);
                        ImGui::Text("Inserted Audio Files:");
                        ImGui::Combo("##Audio In Component", &current_audio, AudioList, static_cast<int>(sList.size()), static_cast<int>(sList.size()));
                        //edit channel group
                        if (AudioArr.size() > current_audio) {
                            ImGui::Text("Inserted Audio Channel Group:");
                            const char* ChList = "None\0Master\0BGM\0SFX";
                            int ChIndex = static_cast<int>(AudioComp[current_audio].mChannelGroup);
                            ImGui::Combo("###test", &ChIndex, ChList);
                            ImGui::PushItemWidth(100.0f);
                            AudioComp[current_audio].mChannelGroup = static_cast<Audio::AudioType>(ChIndex);
                        }
                        //option to add audio to array
                        if (ImGui::Button("Add Audio")) {
                            Audio::AudioPiece nAud;
                            nAud.mAudioPath = audioFileList[currentfile];
                            nAud.mChannelGroup = Audio::AudioType::NONE;
                            nAud.mChannel = 0;
                            nAud.is_Looping = false;
                            nAud.should_play = false;
                            nAud.should_stop = false;
                            nAud.is_Playing = false;
                            AudioArr.push_back(nAud);
                        }
                        ImGui::SameLine();
                        //option to remove audio from array
                        if (ImGui::Button("Remove Audio")) {
                            AudioArr.erase(AudioArr.begin() + current_audio);
                        }
                    }
                }
                //Tag Component
                if (p_ecs.HaveComponent<Tag>(selectedEntity))
                {
                    if (ImGui::CollapsingHeader("TAG", ImGuiTreeNodeFlags_None))
                    {
                        auto& tag = p_ecs.GetComponent<Tag>(selectedEntity).GetTag();
                        char buffer1[256];
                        memset(buffer1, 0, sizeof(buffer1));
                        strcpy_s(buffer1, sizeof(buffer1), tag.c_str());
                        if (ImGui::InputText("Tag", buffer1, sizeof(buffer1)))
                        {
                            tag = std::string(buffer1);
                        }
                    }
                }
                //HUD Component
                if (p_ecs.HaveComponent<HUDComponent>(selectedEntity))
                {
                    if (ImGui::CollapsingHeader("HUD", ImGuiTreeNodeFlags_None))
                    {
                        auto& HUDComp = p_ecs.GetComponent<HUDComponent>(selectedEntity);
                        const char* HUDList = "Static\0Boss Overlay\0Health Bar\0Hunger Bar\0Sanity Bar\0Dash Bar\0Charge Attack\0Text";
                        auto HUDType = HUDComp.GetType();
                        int HUDIndex = static_cast<int>(HUDType);
                        ImGui::Combo("HudType", &HUDIndex, HUDList);
                        ImGui::PushItemWidth(100.0f);
                        HUDComp.SetType(static_cast<HUDComponent::ElementType>(HUDIndex));

                        auto& Offset = HUDComp.GetOffset();
                        ImGui::PushItemWidth(100.0f);
                        ImGui::Text("Offset"); ImGui::SameLine();
                        ImGui::Text("X"); ImGui::SameLine();                    //set a "x" to indicate x-axis
                        ImGui::DragFloat("##OffsetX", (float*)&Offset.x, 0.005f); ImGui::SameLine(); //char name , pass float pointer to position vec2D which hold x and y, the scaling value in imgui
                        ImGui::PushID(1);
                        ImGui::Text("Y"); ImGui::SameLine();
                        ImGui::DragFloat("##OffsetY", (float*)&Offset.y, 0.005f);
                        ImGui::PopID();
                        HUDComp.SetOffset(Offset);
                    }
                }

                //If Entity have Animal Attibutes Component
                if (p_ecs.HaveComponent<AnimalAttributes>(selectedEntity))
                {
                    if (ImGui::CollapsingHeader("Animal Attributes", ImGuiTreeNodeFlags_None)) {
                        auto& animal_attributes = p_ecs.GetComponent<AnimalAttributes>(selectedEntity);
                        ImGui::PushItemWidth(100.0f);
                        ImGui::Text("Max Health"); ImGui::SameLine();
                        if (ImGui::InputDouble("##Max Health", &animal_attributes.maxHealth))
                        {

                            animal_attributes.current_health = glm::clamp(animal_attributes.maxHealth, 0.0, 9999.9999);
                        }

                        ImGui::Text("Max Hunger"); ImGui::SameLine();
                        if (ImGui::InputDouble("##Max Hunger", &animal_attributes.maxHunger))
                        {

                            animal_attributes.current_hunger = glm::clamp(animal_attributes.maxHunger, 0.0, 9999.9999);
                        }

                        ImGui::Text("Max Sanity"); ImGui::SameLine();
                        if (ImGui::InputDouble("##Max Sanity", &animal_attributes.maxSanity))
                        {

                            animal_attributes.current_sanity = glm::clamp(animal_attributes.maxSanity, 0.0, 9999.9999);
                        }
                        ImGui::Text("Current Health"); ImGui::SameLine();
                        if (ImGui::InputDouble("##Current Health", &animal_attributes.current_health))
                        {
                            
                            animal_attributes.current_health = glm::clamp(animal_attributes.current_health, 0.0, animal_attributes.maxHealth);
                        }
       
                        ImGui::Text("Current Hunger"); ImGui::SameLine();
                        if (ImGui::InputDouble("##Current Hunger", &animal_attributes.current_hunger))
                        {

                            animal_attributes.current_hunger = glm::clamp(animal_attributes.current_hunger, 0.0, animal_attributes.maxHunger);
                        }

                        ImGui::Text("Current Sanity"); ImGui::SameLine();
                        if (ImGui::InputDouble("##Current Sanity", &animal_attributes.current_sanity))
                        {

                            animal_attributes.current_sanity = glm::clamp(animal_attributes.current_sanity, 0.0, animal_attributes.maxSanity);
                        }

                        ImGui::Text("Health Decay Rate"); ImGui::SameLine();
                        ImGui::InputDouble("##Health Decay Rate", &animal_attributes.healthDecayRate);

                        ImGui::Text("Hunger Decay Rate"); ImGui::SameLine();
                        ImGui::InputDouble("##Hunger Decay Rate", &animal_attributes.hungerDecayRate);

                        ImGui::Text("Sanity Decay Rate"); ImGui::SameLine();
                        ImGui::InputDouble("##Sanity Decay Rate", &animal_attributes.sanityDecayRate);

                        ImGui::Checkbox("Is Alive", &animal_attributes.is_alive);
                    }
                }

                //If Entity have Item Attibutes Component
                if (p_ecs.HaveComponent<ItemsAttributes>(selectedEntity))
                {
                    if (ImGui::CollapsingHeader("Items Attributes", ImGuiTreeNodeFlags_None)) {
                        auto& item_attributes = p_ecs.GetComponent<ItemsAttributes>(selectedEntity);
                        static const char* ItemTypeNames[] = {
                            "Food",
                            "Medicine",
                            "Toy"
                        };

                        int current = static_cast<int>(item_attributes.type);

                        if (ImGui::Combo("Item Type", &current,
                            ItemTypeNames,
                            IM_ARRAYSIZE(ItemTypeNames)))
                        {
                            item_attributes.type = static_cast<ItemsAttributes::ItemType>(current);
                        }
                        ImGui::PushItemWidth(100.0f);
                        ImGui::Text("gain amount"); ImGui::SameLine();
                        ImGui::InputDouble("##gain amount", &item_attributes.gain_amount);
                    }
                }
                //Draggable
                if (p_ecs.HaveComponent<Draggable>(selectedEntity))
                {
                    if (ImGui::CollapsingHeader("Draggable", ImGuiTreeNodeFlags_None)) {
                        auto& draggable = p_ecs.GetComponent<Draggable>(selectedEntity);
                        ImGui::Checkbox("Is Target", &draggable.IsTarget()); 
                    }
                }

                

                if (ImGui::Button("Delete Component"))
                    ImGui::OpenPopup("Delete Component");

                if (ImGui::BeginPopup("Delete Component"))
                {
                    if (ImGui::MenuItem("Transform") && p_ecs.HaveComponent<Transform>(selectedEntity))
                    {
                        p_ecs.RemoveComponent<Transform>(selectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Sprite") && p_ecs.HaveComponent<Sprite>(selectedEntity))
                    {
                        p_ecs.RemoveComponent<Sprite>(selectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Collider") && p_ecs.HaveComponent<Collider>(selectedEntity))
                    {
                        p_ecs.RemoveComponent<Collider>(selectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("RigidBody") && p_ecs.HaveComponent<RigidBody>(selectedEntity))
                    {
                        p_ecs.RemoveComponent<RigidBody>(selectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Logic") && p_ecs.HaveComponent<Logic>(selectedEntity))
                    {
                        p_ecs.RemoveComponent<Logic>(selectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Audio") && p_ecs.HaveComponent<Audio>(selectedEntity))
                    {
                        p_ecs.RemoveComponent<Audio>(selectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Tag") && p_ecs.HaveComponent<Tag>(selectedEntity))
                    {
                        p_ecs.RemoveComponent<Tag>(selectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("HUD") && p_ecs.HaveComponent<HUDComponent>(selectedEntity))
                    {
                        p_ecs.RemoveComponent<HUDComponent>(selectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Text") && p_ecs.HaveComponent<Text>(selectedEntity))
                    {
                        p_ecs.RemoveComponent<Text>(selectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Draggable") && p_ecs.HaveComponent<Draggable>(selectedEntity))
                    {
                        p_ecs.RemoveComponent<Draggable>(selectedEntity);
                        ImGui::CloseCurrentPopup();
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

            float f1 = p_Audio->GetVolumeByChannel(p_Audio->GetMasterChannelGroup());
            float f4 = p_Audio->GetVolumeByChannel(p_Audio->GetBGMChannelGroup());
            float f5 = p_Audio->GetVolumeByChannel(p_Audio->GetSFXChannelGroup());
            
            for (Entity i = 0; i < p_ecs.GetTotalEntities(); ++i) 
            {
                if (p_ecs.HaveComponent<Audio>(i)) 
                {
                    f1 = p_ecs.GetComponent<Audio>(i).GetMasterVolume();
                    f4 = p_ecs.GetComponent<Audio>(i).GetBGMVolume();
                    f5 = p_ecs.GetComponent<Audio>(i).GetSFXVolume();
                }
            }
            //set voulume slider
            ImGui::SliderFloat("Master Volume", &f1, 0.0f, 1.0f, "Min - Max %.3f");
            p_Audio->SetVolumeByChannel(p_Audio->GetMasterChannelGroup(), f1);

            ImGui::SliderFloat("BGM vol", &f4, 0.0f, 1.0f, "Min - Max %.3f");
            if (f4 > f1)
            {
                f4 = f1;
            }
            p_Audio->SetVolumeByChannel(p_Audio->GetBGMChannelGroup(), f4);


            ImGui::SliderFloat("SFX vol", &f5, 0.0f, 1.0f, "Min - Max %.3f");
            if (f5 > f1)
            {
                f5 = f1;
            }
            p_Audio->SetVolumeByChannel(p_Audio->GetSFXChannelGroup(), f5);

            for (Entity i = 0; i < p_ecs.GetTotalEntities(); ++i)
            {
                if (p_ecs.HaveComponent<Audio>(i))
                {
                    p_ecs.GetComponent<Audio>(i).SetMasterVolume(f1);
                    p_ecs.GetComponent<Audio>(i).SetBGMVolume(f4);
                    p_ecs.GetComponent<Audio>(i).SetSFXVolume(f5);
                }
            }
            ImGui::End();
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
        auto const& dir_entry = std::filesystem::directory_iterator( in );
        for (const auto& entry : dir_entry)
        {
            if (entry.path().extension() == ".png") {

            }
        }
    }
}
