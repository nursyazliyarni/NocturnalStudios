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
#include <vector>
#include <iostream>



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

    // Save initial windowed size/pos
    glfwGetWindowPos(m_win, &m_windowedX, &m_windowedY);
    glfwGetWindowSize(m_win, &m_windowedW, &m_windowedH);

    if (desc.fullscreen) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (monitor && mode) {
            glfwSetWindowMonitor(m_win, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            m_isFullscreen = true;
        }
    } else {
        m_isFullscreen = false;
    }

	//enable file drop callback
    glfwSetWindowUserPointer(m_win, this);
    glfwSetDropCallback(m_win, Window::glfwDropCallback);

    return true;
}

void Window::destroy() {
    if (m_win) { glfwDestroyWindow(m_win); m_win = nullptr; }
    glfwTerminate();
}

bool Window::shouldClose() const { return m_win && glfwWindowShouldClose(m_win); }
void Window::poll() { glfwPollEvents(); }
void Window::swap() { glfwSwapBuffers(m_win); }


//void Window::size(int& w, int& h) const {
//    if (!m_win) { w = h = 0; return; }
//    glfwGetFramebufferSize(m_win, &w, &h);
//}


void Window::setTitleFps(const std::string& base, int fps) {
    if (!m_win) return;
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s | FPS: %d", base.c_str(), fps);
    glfwSetWindowTitle(m_win, buf);
}

void Window::getWindowSize(int& w, int& h) const {
    if (!m_win) { w = h = 0; return; }

    // If fullscreen, report the monitor's current video mode
    if (GLFWmonitor* mon = glfwGetWindowMonitor(m_win)) {
        if (const GLFWvidmode* mode = glfwGetVideoMode(mon)) {
            w = mode->width;
            h = mode->height;
            return;
        }
    }

    // Windowed (or fallback)
    glfwGetWindowSize(m_win, &w, &h);
}

void Window::getFramebufferSize(int& w, int& h) const {
    if (!m_win) { w = h = 0; return; }
    glfwGetFramebufferSize(m_win, &w, &h);
}

void Window::setFullscreen(bool fs) {
    if (!m_win) return;
    if (fs == m_isFullscreen) return;

    if (fs) {
        // Save windowed placement before going fullscreen
        glfwGetWindowPos(m_win, &m_windowedX, &m_windowedY);
        glfwGetWindowSize(m_win, &m_windowedW, &m_windowedH);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (monitor && mode) {
            glfwSetWindowMonitor(m_win, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            m_isFullscreen = true;
        }
    } else {
        // Restore windowed placement
        glfwSetWindowMonitor(m_win, nullptr, m_windowedX, m_windowedY, m_windowedW, m_windowedH, 0);
        m_isFullscreen = false;
    }
}

void Window::toggleFullscreen() {
    setFullscreen(!m_isFullscreen);
}

std::vector<std::string> Window::consumeDroppedFiles()
{
    std::vector<std::string> out;
    out.swap(m_droppedFiles);   //move & clear internal vector
    return out;
}

void Window::glfwDropCallback(GLFWwindow* win, int count, const char** paths)
{
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
    if (!self) return;
    self->onFilesDropped(count, paths);
}

void Window::onFilesDropped(int count, const char** paths)
{
    std::cout << "[Window] onFilesDropped: count = " << count << "\n";
    for (int i = 0; i < count; i++)
    {
        if (paths[i])
            m_droppedFiles.emplace_back(paths[i]);
    }
}

