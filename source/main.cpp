#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <cstdio>

int main() {
    if (!glfwInit()) { std::fprintf(stderr, "GLFW init failed\n"); return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* win = glfwCreateWindow(1280, 720, "NocturnalStudios", nullptr, nullptr);
    if (!win) { std::fprintf(stderr, "Window creation failed\n"); glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1); // vsync

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::fprintf(stderr, "GLEW init error\n"); return -1; }
    glGetError(); // clear harmless GL error from glewInit

    std::printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    std::printf("GPU renderer:   %s\n", glGetString(GL_RENDERER));


    auto last = std::chrono::high_resolution_clock::now();
    int frames = 0; double acc = 0.0;

    while (!glfwWindowShouldClose(win)) {
        auto now = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration<double>(now - last).count();
        last = now; acc += dt; frames++;

        if (acc >= 1.0) {
            char title[64]; std::snprintf(title, sizeof(title), "NocturnalStudios | FPS: %d", frames);
            glfwSetWindowTitle(win, title); frames = 0; acc = 0.0;
        }

        glClearColor(0.1f,0.1f,0.12f,1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
