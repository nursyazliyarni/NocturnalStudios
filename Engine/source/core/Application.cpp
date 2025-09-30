/*!------------------------------------------------------------------------------------------------
    File: Application.cpp
    Project: NocturnalStudio

    This file contains ...

    Primary Author:
         Rishika Bimal Attodi
        - Brief

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

namespace {
    constexpr float TWO_PI = 2.0f * glm::pi<float>();
    constexpr int   CIRCLE_SEGMENTS = 64;
	using Clock = std::chrono::high_resolution_clock;
}

Application::Application() = default;

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

    // Prime the window so there’s no white flash before the first real frame
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

    // Initialize ImGui Manager
    // NOTE: ImGuiManager owns the Viewport pass; it will call Renderer to draw bg + shapes into an FBO.
    //       We just pass references here. No drawing is done in this file anymore.
    //std::vector<char const*> vs{ "assets/textures/DGPEN_LOGO.png" };


    std::vector<char const*> vs{
    "assets/textures/DGPEN_LOGO.png",   // [0] background ONLY
    "assets/textures/drink.png",       // [1]
    "assets/textures/cupcake.png",       // [2]
    "assets/textures/placeholder.png",  // [3]
    "assets/textures/placeholder2.png"  // [4]
    };

    Texture  txt_obj(vs);
    Renderer bg_render("assets/shaders/background.vert", "assets/shaders/background.frag");

    Input      game_input_obj;
    Game_Loop  game_loop_obj;
    ImGuiManager imGuiManager(m_window, bg_render, txt_obj, game_input_obj);

    game_input_obj.process_user_input(m_window.handle()); // installs callbacks + sets user pointer

    // make Input’s scale array match the number of textures
    game_input_obj.initSpriteScales(txt_obj.count());

    // 4) simple loop
    while (!m_window.shouldClose()) {
        m_window.poll();

        double dt = game_loop_obj.calculate_DT();          // our deltaTime
        double total_time = game_loop_obj.get_accum_GT();  // accumulated game time
        game_loop_obj.display_FPS(m_window.handle(), c.window.title, dt); // display FPS

        // Main Game Loop Update Here
        game_loop_obj.update(m_window.handle());
        //true->get normalized value, false->unnormalized value
        game_input_obj.update_mouse_pos(m_window.handle(),true,c.window.width,c.window.height);

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

    m_window.destroy();
    return 0;
}
