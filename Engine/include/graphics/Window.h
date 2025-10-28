/*!------------------------------------------------------------------------------------------------
    File: Window.h
    Project: NocturnalStudio

   This header declares a minimal OS window wrapper around GLFW:
    - Creates/destroys an OpenGL-capable window and context
    - Manages vsync, event polling, buffer swapping, and window title FPS updates
    - Exposes the raw GLFWwindow* and immutable WindowDesc for other systems

    Primary Author:
         Rishika Bimal Attodi 
        -- Implemented GLFW-based window lifecycle and utility helpers


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include <string>

struct GLFWwindow; // fwd

struct WindowDesc {
    int         width = 1280;
    int         height = 720;
    std::string title = "NocturnalStudios";
    bool        vsync = true;
};

class Window {
public:
    Window() = default;
    ~Window();

    bool create(const WindowDesc& desc);
    void destroy();

    bool shouldClose() const;
    void poll();
    void swap();

    void setTitleFps(const std::string& base, int fps);
    GLFWwindow* handle() const { return m_win; }
    const WindowDesc& desc() const { return m_desc; }

   /* void size(int& w, int& h) const;
    GLFWwindow* native() const { return m_win; }*/


private:
    GLFWwindow* m_win = nullptr;
    WindowDesc  m_desc{};
};
