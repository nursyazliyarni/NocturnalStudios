//gsm.h
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include "graphics/Renderer.h"


class ECS;

struct DrawItem {
    GLuint tex = 0;         // GL texture id
    float x = 0, y = 0, w = 0, h = 0, angle = 0; // normalized
};

struct ClickRegion {
    float x, y, w, h;          // normalized rect
    std::string gotoScene;     // empty if not a goto
    bool quit = false;         // quit app?
};

struct Scene {
    GLuint background = 0;
    std::vector<DrawItem> items;
    std::vector<ClickRegion> clicks;
};

class TextureStore {
public:
    GLuint get(const std::string& path); // loads on first use
    void clear();
private:
    std::unordered_map<std::string, GLuint> cache;
};

class GameStateManager {
public:
    bool load(const char* prefabsJsonPath, const char* scenesJsonPath, TextureStore& tex);

    void requestScene(const std::string& name);
    const std::string& current() const { return currentScene; }

    void handleKeyPress(int key); // map numeric keys/ESC to scenes
    void render(Renderer& r, int w, int h) const;

    bool handleClick(float nx, float ny);  // mouse click in normalized coords
    bool popQuitRequested();               // returns & clears quit flag

    enum class AppState { Menu, Playing, Paused };

    bool isPlaying() const { return app == AppState::Playing; }
    bool isPaused()  const { return app == AppState::Paused; }

    void startGame();
    void goToMenu();
    // (optional) void pause(); void resume();

    // Expose a safe key->scene helper if you don’t want Input.cpp touching keyMap directly:
    bool switchToSceneForKey(int glfwKey);

    void setECS(ECS* e) { ecs_ = e; }


private:
    std::unordered_map<std::string, Scene> scenes;
    std::unordered_map<int, std::string> keyMap; // GLFW_KEY_* to scene name
    //std::string backKey = "MainMenu";
    //std::string currentScene;

    AppState app{ AppState::Menu };                              // current mode
    std::string backKey;                                         // ESC destination (from JSON)
    bool requestedQuit{ false };                                  // quit flag
    std::string currentScene;

    ECS* ecs_ = nullptr;

};
