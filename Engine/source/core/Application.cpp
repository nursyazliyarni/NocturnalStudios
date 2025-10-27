/*!------------------------------------------------------------------------------------------------
    File: Application.cpp
    Project: NocturnalStudio

	This file contains the main application loop that initializes the window creation, OpenGL initialization,
	sprite rendering, input handling, drag and drop interations, frame updates, ImGui systems.

    Primary Author:
         
        - Brief

    Primary Author:
        Rishika Bimal Attodi 
        - Implemented main loop scaffolding: window creation, OpenGL setup, 
        sprite batching

    Secondary Author:
        Yu Xuan
        -  Background

    Secondary Authod
        Xiang Yang
        - FBO

    Secondary Author:
		Chang Qi Xuan Celest (35%)
		- Integrated drag and drop functionality for sprites.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
// init(), run(), shutdown()
#include "core/Application.h"
#include "core/Config.h"
#include <GLFW/glfw3.h>            // glViewport/glClearColor/glClear
#include <cmath>                   // cosf, sinf
#include <glm/gtc/constants.hpp>   // glm::pi<float>()
#include <chrono>
#include "math/Vector.h"
#include "core/ImGUIManager.h"
#include "./scene/Prefab.h"
#include "physics/AABB.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace {
    constexpr float TWO_PI = 2.0f * glm::pi<float>();
    constexpr int   CIRCLE_SEGMENTS = 64;
	using Clock = std::chrono::high_resolution_clock;
}

bool isDragging = false; //is a sprite being dragged
int draggedSpriteIndex = -1; //index into sprite_vector
int duplicateIndex = -1; //where clone is in the vector
Vector2D dragOffset(0.0f, 0.0f); //mouse offset from top-left of sprite

//Application::Application() = default;
Application::Application() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //enable memory leak check at program exit
}

int Application::run() {
    // 1) read JSON (with safe defaults)
    //const WindowConfig c = Config::LoadWindowConfig("assets/config.json");
    const GameConfig c = loadConfig("assets/config.json");

    // 2) create the window
    WindowDesc wd;
    wd.width = c.window.width;
    wd.height = c.window.height;
    wd.title = c.window.title;
    wd.vsync = c.window.vsync;

    if (!m_window.create(wd)) return -1;

    // Prime the window so there�s no white flash before the first real frame
    glViewport(0, 0, wd.width, wd.height);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    m_window.swap();


    // set up a simple 2D projection (pixel coords, origin = top-left) for the OS window
    int currW = wd.width, currH = wd.height;
    glfwGetFramebufferSize(m_window.handle(), &currW, &currH);
    glViewport(0, 0, currW, currH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, currW, currH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // sensible 2D state for the OS window (ImGui draws here later)
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    //Register GameObject Components
    RegisterAllComponents();

    // Initialize ImGui Manager
    // NOTE: ImGuiManager owns the Viewport pass; it will call Renderer to draw bg + shapes into an FBO.
    //       We just pass references here. No drawing is done in this file anymore.

    std::vector<char const*> vs{
    "assets/textures/DGPEN_LOGO.png",   // [0] background ONLY
    "assets/textures/drink.png",       // [1]
    "assets/textures/cupcake.png",       // [2]
    "assets/textures/placeholder.png",  // [3]
    "assets/textures/placeholder2.png",  // [4]
	"assets/textures/catate.png",    // [5]
	"assets/textures/catate2.png"   // [6]
    };

    Texture  txt_obj(vs);
    Renderer bg_render("assets/shaders/background.vert", "assets/shaders/background.frag");


    Input      game_input_obj(m_window.handle());

    game_input_obj.initSpriteScales(txt_obj.get_totalTexture());

    game_input_obj.initSpriteScales(txt_obj.get_totalTexture());
    game_input_obj.initSpriteRotations(txt_obj.get_totalTexture());


    Physics    game_physics_obj;
    Game_Loop  game_loop_obj(m_window.handle());
    ImGuiManager imGuiManager(m_window, bg_render, txt_obj, game_input_obj, game_physics_obj);
    PrefabManager prefab_man;

    // make Input�s scale array match the number of textures
    //game_input_obj.initSpriteScales(txt_obj.count());

    //to demonstrate cloning and rendering of gameobjects, this willl be moved into levelmanager
    int catnum{}, maxcats{ 3 };
    GameObject AlphaCat{ *prefab_man["Cat"] };
    std::vector<std::unique_ptr<GameObject>> cats{};

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR: Could not initialize FreeType Library!" << std::endl;
        return -1;
    }
    else {
        std::cout << "FreeType initialized successfully." << std::endl;
    }

    // Load the font
    FT_Face face;
    if (FT_New_Face(ft, "assets/fonts/NanumBrushScript-Regular.ttf", 0, &face)) {
        std::cerr << "Failed to load font file!\n";
    }
    else {
        std::cout << "Font loaded successfully!\n";
        FT_Set_Pixel_Sizes(face, 0, 48); // Set pixel size for the font (48px height)
        std::cout << "Font family: " << face->family_name << std::endl;
    }

    // 4) simple loop
    while (!m_window.shouldClose()) {
        m_window.poll();

        //spawns 3 cats
        if (catnum<maxcats)
        {
            cats.push_back(std::make_unique<GameObject>(AlphaCat));
            cats[catnum]->Render(bg_render);
            logger.log(std::string{"cat "+std::to_string(catnum)});
            catnum++;
        }
        if (catnum < 4 && glfwGetKey(m_window.handle(), GLFW_KEY_O) == GLFW_PRESS)maxcats = 2500;
            
        double dt = game_loop_obj.calculate_DT();          // our deltaTime
        double total_time = game_loop_obj.get_accum_GT();  // accumulated game time
        game_loop_obj.display_FPS(m_window.handle(), c.window.title, dt); // display FPS
        game_input_obj.update_mouse_pos(m_window.handle());
        game_input_obj.GET_mouse_delta();


        //---------------------DRAG AND DROP---------------------------
        double mouseX, mouseY;
        glfwGetCursorPos(m_window.handle(), &mouseX, &mouseY);
        Vector2D mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));

        auto& sprites = bg_render.get_sprite_vector();
        std::vector<int> frontIdxs; //indices to draw on top

        if (!isDragging) {
            if (glfwGetMouseButton(m_window.handle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                for (int i = 0; i < (int)sprites.size(); i++) {
                    if (i != 0 && i != 1) continue; //only drink & cupcake are draggable
                    const auto& spr = sprites[i];

                    float x = spr.x * currW;
                    float y = spr.y * currH;
                    float w = spr.w * currW;
                    float h = spr.h * currH;

                    if (isPointInAABB(mousePos, { x, y }, { w, h })) {
                        //create duplicate and bring it to front
                        sprites.push_back(spr);
                        duplicateIndex = (int)sprites.size() - 1;
                        draggedSpriteIndex = i;
                        isDragging = true;
                        dragOffset = mousePos - Vector2D(x, y);

                        frontIdxs.push_back(sprites[duplicateIndex].idx); //ensure duplicate is drawn on top
                        break;
                    }
                }
            }
        }
        else { //isDragging
            if (glfwGetMouseButton(m_window.handle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
                if (duplicateIndex >= 0 && duplicateIndex < (int)sprites.size()) {
                    auto& dup = sprites[duplicateIndex];
                    frontIdxs.push_back(sprites[duplicateIndex].idx);

                    //check drop target
                    for (int i = 0; i < (int)sprites.size(); ++i) {
                        if (i == draggedSpriteIndex || i == duplicateIndex) continue;
                        auto& target = sprites[i];
                        //target is a cat
                        if (target.idx == 3 || target.idx == 4) {
                            float tx = target.x * currW;
                            float ty = target.y * currH;
                            float tw = target.w * currW;
                            float th = target.h * currH;

                            if (isPointInAABB(Vector2D(dup.x * currW, dup.y * currH), { tx, ty }, { tw, th })) {
                                if(target.idx==3)target.idx=5; //change cat A texture to cat A ate
                                if(target.idx==4)target.idx=6; //change cat B texture to cat B ate
                            }
                        }
                    }

                    // remove duplicate
                    sprites.erase(sprites.begin() + duplicateIndex);
                }

                isDragging = false;
                draggedSpriteIndex = -1;
                duplicateIndex = -1;
                frontIdxs.clear();
            }
            else {
                // move duplicate with mouse
                auto& spr = sprites[duplicateIndex];
                float newX = mousePos.x() - dragOffset.x();
                float newY = mousePos.y() - dragOffset.y();
                spr.x = newX / currW;
                spr.y = newY / currH;

                frontIdxs.clear();
                frontIdxs.push_back(spr.idx); // always draw duplicate on top
            }
        }

        // draw sprites: first behind pass (everything except front), then front pass
        bg_render.drawAllSprites(txt_obj, frontIdxs, true, currW, currH);   // behind pass
        bg_render.drawAllSprites(txt_obj, frontIdxs, false, currW, currH);  // front pass
        //---------------------END DRAG AND DROP---------------------------

        // handle resize: update viewport/projection if size changed (no callbacks)

        // handle OS-window resize: keep a sane viewport/projection for the UI backdrop
        int fbw, fbh;
        glfwGetFramebufferSize(m_window.handle(), &fbw, &fbh);
        if (fbw != currW || fbh != currH) {
            currW = fbw; currH = fbh;
            glViewport(0, 0, currW, currH);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, currW, currH, 0, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
        }

        // Clear the OS window behind the UI (the actual scene is drawn inside the Viewport FBO)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, currW, currH);
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		auto imGuiStart = Clock::now();
        // Build & render all ImGui (Viewport will render bg + shapes internally via Renderer)
        Gui::updateImGuiManager(dt, imGuiManager);
		auto imGuiEnd = Clock::now();
		double imGuiTime = std::chrono::duration<double>(imGuiEnd - imGuiStart).count();

		imGuiManager.setSystemTimings(imGuiTime, 0.0);

        m_window.swap();
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    m_window.destroy();
    return 0;
}
