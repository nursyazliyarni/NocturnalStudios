// init(), run(), shutdown()
#include "core/Application.h"
#include "core/Config.h"
#include <GLFW/glfw3.h> // glViewport/glClearColor/glClear

Application::Application() = default;

int Application::run() {
    // 1) read JSON (with safe defaults)
   //const WindowConfig c = Config::LoadWindowConfig("assets/config.json");
    GameConfig cfg = loadConfig("assets/config.json");

    // 2) create the window
    WindowDesc wd;
    wd.width = cfg.window.width;
    wd.height = cfg.window.height;
    wd.title = cfg.window.title;
    wd.vsync = cfg.window.vsync;

    if (!m_window.create(wd)) return -1;

    // 3) FPS count
    double t0 = glfwGetTime();
    int frames = 0;

    // 4) simple loop
    while (!m_window.shouldClose()) {
        m_window.poll();

        ++frames;
        double t = glfwGetTime();
        if (t - t0 >= 1.0) {
            m_window.setTitleFps(cfg.window.title, frames);
            frames = 0;
            t0 = t;
        }

        glViewport(0, 0, wd.width, wd.height);
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_window.swap();
    }

    m_window.destroy();
    return 0;
}
