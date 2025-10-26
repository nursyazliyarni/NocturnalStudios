/*!------------------------------------------------------------------------------------------------
    File: Window.cpp
    Project: NocturnalStudio

    This file contains ...

    Primary Author:
         Rishika Bimal Attodi
        - Brief

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "graphics/Window.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>

Window::~Window() { destroy(); }

bool Window::create(const WindowDesc& desc) {
    m_desc = desc;

    if (!glfwInit()) {
        std::fprintf(stderr, "GLFW init failed\n");
        return false;
    }

    // Keep it simple: no version/profile hints.
    m_win = glfwCreateWindow(m_desc.width, m_desc.height, m_desc.title.c_str(), nullptr, nullptr);
    if (!m_win) {
        std::fprintf(stderr, "Window creation failed\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_win);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::fprintf(stderr, "Failed to initialize GLEW\n");
    }
    glfwSwapInterval(m_desc.vsync ? 1 : 0);
    return true;
}

void Window::destroy() {
    if (m_win) { glfwDestroyWindow(m_win); m_win = nullptr; }
    glfwTerminate();
}

bool Window::shouldClose() const { return m_win && glfwWindowShouldClose(m_win); }
void Window::poll() { glfwPollEvents(); }
void Window::swap() { glfwSwapBuffers(m_win); }

void Window::setTitleFps(const std::string& base, int fps) {
    if (!m_win) return;
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s | FPS: %d", base.c_str(), fps);
    glfwSetWindowTitle(m_win, buf);
}
