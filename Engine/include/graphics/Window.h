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

private:
    GLFWwindow* m_win = nullptr;
    WindowDesc  m_desc{};
};
