// init(), run(), shutdown()
#include "core/Application.h"
#include "core/Config.h"
#include <GLFW/glfw3.h> // glViewport/glClearColor/glClear

Application::Application() = default;

int Application::run() {
    // 1) read JSON (with safe defaults)
    const GameConfig g = loadConfig("assets/config.json");
    const WindowConfig& c = g.window;

    // 2) create the window
    WindowDesc wd;  
    wd.width = c.width;
    wd.height = c.height;
    wd.title = c.title;
    wd.vsync = c.vsync;

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
            m_window.setTitleFps(c.title, frames);
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
