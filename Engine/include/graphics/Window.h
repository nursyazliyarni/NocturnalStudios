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

    Secondary Authors:
        Lau Xiang Yang (5%)
        - Added fullscreen toggle and size queries

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include <string>
#include <vector>

struct GLFWwindow; // fwd

/**
 * WindowDesc - immutable descriptor used to configure a Window at creation.
 */
struct WindowDesc {
    int         width = 1280;            /**< Initial window width in logical units. */
    int         height = 720;            /**< Initial window height in logical units. */
    std::string title = "NocturnalStudios"; /**< Window title shown in the title bar. */
    bool        vsync = true;            /**< Enable vertical sync (swap interval = 1) when true. */
    bool        fullscreen = true;       /**< Start the window in fullscreen on the primary monitor. */
};

/**
 * Window - minimal GLFW-backed OS window wrapper.
 *
 * Responsibilities:
 *  - Create / destroy an OpenGL-capable window and context
 *  - Manage vsync, event polling, buffer swapping
 *  - Provide window size and framebuffer size queries
 *  - Toggle or set fullscreen and preserve/restores windowed placement
 */
class Window {
public:
    Window() = default;
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    ~Window();

    /**
     * create - initialize GLFW and create the window according to desc.
     * @param desc Configuration used to create the window (copied).
     * @return true on success, false on failure (prints errors to stderr).
     *
     * Notes:
     *  - Makes the created context current and initializes GLEW.
     *  - Applies vsync from desc.vsync.
     *  - If desc.fullscreen is true, the window is switched to the primary monitor's video mode.
     */
    bool create(const WindowDesc& desc);

    /**
     * destroy - destroy the GLFW window and terminate GLFW.
     * Safe to call multiple times.
     */
    void destroy();

    /**
     * shouldClose - query whether the OS/window has requested closure.
     * @return true if the window should close.
     */
    bool shouldClose() const;

    /**
     * poll - process pending OS/window events (glfwPollEvents).
     */
    void poll();

    /**
     * swap - swap the front/back buffers (glfwSwapBuffers).
     */
    void swap();

    /**
     * setTitleFps - set the window title to "base | FPS: <fps>".
     * @param base Base title string.
     * @param fps  Integer frames-per-second value to append.
     */
    void setTitleFps(const std::string& base, int fps);

    /**
     * handle - access the raw GLFWwindow* (may be nullptr).
     * @return raw GLFWwindow pointer.
     */
    GLFWwindow* handle() const { return m_win; }

    /**
     * desc - access the WindowDesc used to create this window.
     * @return const reference to the stored WindowDesc.
     */
    const WindowDesc& desc() const { return m_desc; }


    std::vector<std::string> consumeDroppedFiles(); //retrieve any files dropped onto the window

    // Size queries
    /**
     * getWindowSize - retrieve the logical client area size in screen coordinates.
     * @param w Out: width in logical units.
     * @param h Out: height in logical units.
     *
     * If the window is fullscreen this reports the monitor's current video mode size.
     */
    void getWindowSize(int& w, int& h) const;

    /**
     * getFramebufferSize - retrieve the framebuffer size in pixels (useful for glViewport).
     * @param w Out: pixel width of the framebuffer.
     * @param h Out: pixel height of the framebuffer.
     */
    void getFramebufferSize(int& w, int& h) const;
    
    /**
     * toggleFullscreen - flip between fullscreen and windowed modes.
     * Preserves and restores the previous windowed position/size.
     */
    void toggleFullscreen();

    /**
     * setFullscreen - explicitly set fullscreen state.
     * @param fs true to enter fullscreen, false to restore windowed mode.
     */
    void setFullscreen(bool fs);

    /**
     * isFullscreen - query whether the window is currently fullscreen.
     * @return true if fullscreen, false otherwise.
     */
    bool isFullscreen() const { return m_isFullscreen; }


private:
    GLFWwindow* m_win = nullptr;
    WindowDesc  m_desc{};

    bool m_isFullscreen{ false };
    int  m_windowedX{ 100 }, m_windowedY{ 100 }, m_windowedW{ 1280 }, m_windowedH{ 720 };

    std::vector<std::string> m_droppedFiles;
    static void glfwDropCallback(GLFWwindow* win, int count, const char** paths);
    void onFilesDropped(int count, const char** paths);
};
