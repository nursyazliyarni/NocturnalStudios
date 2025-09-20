// init(), run(), shutdown()
#include "core/Application.h"
#include "core/Config.h"
#include <GLFW/glfw3.h>        // glViewport/glClearColor/glClear
#include <cmath>               // cosf, sinf
#include <glm/gtc/constants.hpp>   // glm::pi<float>()

namespace {
    constexpr float TWO_PI = 2.0f * glm::pi<float>();
    constexpr int   CIRCLE_SEGMENTS = 64;
}

Application::Application() = default;

int Application::run() {
    // 1) read JSON (with safe defaults)
    const WindowConfig c = Config::LoadWindowConfig("assets/config.json");

    // 2) create the window
    WindowDesc wd;
    wd.width = c.width;
    wd.height = c.height;
    wd.title = c.title;
    wd.vsync = c.vsync;

    if (!m_window.create(wd)) return -1;

    // set up a simple 2D projection (pixel coords, origin = top-left)
    int currW = wd.width, currH = wd.height;
    glfwGetFramebufferSize(m_window.handle(), &currW, &currH);
    glViewport(0, 0, currW, currH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, currW, currH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // sensible 2D state
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 3) FPS count
    //double t0 = glfwGetTime();
    //int frames = 0;

    Input game_input_obj;
    Game_Loop game_loop_obj;

    std::vector<char const*> vs{ "assets/textures/Circle.png"};
    Texture txt_obj(vs);

    // build and compile my background shader program
    Shader bg_Shader("assets/shaders/background.vert", "assets/shaders/background.frag"); 

    // 4) simple loop
    while (!m_window.shouldClose()) {
        m_window.poll();

        double dt = game_loop_obj.calculate_DT(); //our deltaTime
        double total_time = game_loop_obj.get_accum_GT(); //our accumulated game time
        game_loop_obj.display_FPS(m_window.handle(), c.title, dt); //display game FPS

        //Main Game Loop Update Here
        game_loop_obj.update(m_window.handle());

        /*++frames;
        double t = glfwGetTime();
        if (t - t0 >= 1.0) {
            m_window.setTitleFps(c.title, frames);
            frames = 0;
            t0 = t;
        }*/

        // handle resize: update viewport/projection if size changed (no callbacks)
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

        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw some visible geometry (immediate mode)

        // RGB triangle positioned by window proportions (no magic numbers)
        glLoadIdentity();
        glBegin(GL_TRIANGLES);
        glColor3f(1.f, 0.f, 0.f);  glVertex2f(currW * 0.50f, currH * 0.20f); // top
        glColor3f(0.f, 1.f, 0.f);  glVertex2f(currW * 0.20f, currH * 0.80f); // left
        glColor3f(0.f, 0.f, 1.f);  glVertex2f(currW * 0.80f, currH * 0.80f); // right
        glEnd();

        // filled blue circle on the RIGHT side, positioned by window size
        {
            float r = currH * 0.10f;   // radius = 10% of height
            float margin = currW * 0.05f;   // gap from right = 5% of width
            float cx = currW - (r + margin);
            float cy = currH * 0.20f;   // 20% from top
            if (cx < r) cx = r;             // keep inside if very narrow

            glBegin(GL_TRIANGLE_FAN);
            glColor3f(0.f, 0.5f, 1.f);
            glVertex2f(cx, cy); // center
            for (int i = 0; i <= CIRCLE_SEGMENTS; ++i) {
                float a = (i * TWO_PI) / CIRCLE_SEGMENTS;
                glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
            }
            glEnd();
        }

        // simple white outline box (debug-draw) using proportions
        {
            float pad = currW * 0.02f;      // 2% padding
            float bw = currW * 0.16f;      // 16% width
            float bh = currH * 0.17f;      // 17% height
            glBegin(GL_LINE_LOOP);
            glColor3f(1.f, 1.f, 1.f);
            glVertex2f(pad, pad);
            glVertex2f(pad + bw, pad);
            glVertex2f(pad + bw, pad + bh);
            glVertex2f(pad, pad + bh);
            glEnd();
        }

        m_window.swap();
    }

    m_window.destroy();
    return 0;
}
