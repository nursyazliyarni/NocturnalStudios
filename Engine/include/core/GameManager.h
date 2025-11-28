/*!------------------------------------------------------------------------------------------------
    File: LevelManager.h
    Project: NocturnalStudio

    This file contains ...

    Primary Author:
         Name (%)
        -Brief

    Secondary Author:
         Rishika Bimal Attodi (%)
        -State integration for the play and exit button and scene switching via keys

        Lau Xiang Yang (2%)
        - Implemented pause and resume functionality in GameManager

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

//gsm.h
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include "graphics/Renderer.h"
#include "Animation/AnimationSystem.h"
#include "input/Input.h"
#include "physics/Collision.h"

#include <functional>

class TextureStore;
class ECS;
class Animal_Logic;
class ResourceManager;

struct DrawItem {
    std::string name; std::string item_type;
    GLuint tex = 0;
    //float x = 0, y = 0, w = 0, h = 0, angle = 0; // normalized
    //float health_bar, hunger_bar, sanity_bar;
    //float slower_decreasing_rate, normal_decreasing_rate, faster_decreasing_rate, penalty_decreasing_rate;
    bool still_alive, is_draggable, is_dragging;

    EntityID entityID = INVALID_ENTITY_ID;
	//collider info
    bool  hasCollider = true;
    float colOffsetX = 0.0f;
    float colOffsetY = 0.0f;
    float colW = 0.0f;
    float colH = 0.0f;

    float acc_timer = 0.0f;              // For flamingo viewing timer
    float staring_countdown = 10.0f;     // For seal staring countdown
};

struct ClickRegion {
    float x, y, w, h;          // normalized rect
    std::string gotoScene;     // empty if not a goto
    bool quit = false;         // quit app?
    bool  start{ false };
    std::string name;
};

struct Scene {
    GLuint background = 0;
    std::vector<DrawItem> items;
    std::vector<ClickRegion> clicks;
};

class TextureStore {
public:
    GLuint get(const std::string& path); // loads on first use
    std::string find(const GLuint id) const;
    void clear();
private:
    std::unordered_map<std::string, GLuint> cache;
};

class GameManager {
public:
    bool checkForSceneChange();

    bool load(const char* prefabsJsonPath, const char* scenesJsonPath, TextureStore& tex);
    bool saveScene( std::string scenename, const char* scenesJsonPath, TextureStore& tex);

    void requestScene(const std::string& name);
    const std::string& current() const { return currentScene; }

    void handleKeyPress(int key); // map numeric keys/ESC to scenes
    void render(Renderer& r, Input& input, GLFWwindow* window, int w, int h) ;

    void renderWithHUD(Renderer& r,
        Input& input,
        GLFWwindow* window,
        int w, int h,
        double dt,
        ResourceManager& resourceManager,
        bool showFPS);

    void imGUIlessRender(Renderer& r, Input& input, GLFWwindow* window, int w, int h);

    bool handleClick(float nx, float ny);  // mouse click in normalized coords
    bool popQuitRequested();               // returns & clears quit flag

    enum class AppState { Menu, Playing, Paused };

    bool isPlaying() const { return app == AppState::Playing; }
    bool isPaused()  const { return app == AppState::Paused; }

    void startGame();
    void goToMenu();
    // (optional) void pause(); void resume();

    // Expose a safe key->scene helper if you dont want Input.cpp touching keyMap directly:
    bool switchToSceneForKey(int glfwKey);

    GameManager(ECS& ecs) :ecs_{ ecs } {}
    void update(float dt,Renderer& renderer,TextureStore& tex);

	ECS& getECS() { return ecs_; }
    std::unique_ptr<DrawItem>& get_duplicate_item();
    /**
     * @brief Pauses the game, changing the application state to Paused.
     * 
     */
    void pauseGame();

    /**
     * @brief Resumes the game, changing the application state to Playing.
     * 
     */
    void resumeGame();

    /**
     * Reload the current scene from the JSON file
     * This resets all game state to initial values
     */
    bool reloadCurrentScene(TextureStore& tex);

    void resetAllScenesToInitial();

    // Camera controls (per scene)
    //void setCameraEnabled(const std::string& scene, bool on);
    //void setCameraCenter(const std::string& scene, float cx, float cy);
    //void setCameraZoom(const std::string& scene, float z);
    //void focusOnEntity(const std::string& scene, const std::string& entityName);

    Scene* getCurrentScene();
    std::unordered_map<std::string, Scene>& get_scenes_map();

    void setSceneChangedCallback(std::function<void(const std::string&)> cb) { onSceneChanged = std::move(cb); }

    std::function<void()> quitCallback;


private:
    std::unordered_map<std::string, Scene> scenes;
    std::unordered_map<std::string, Scene> initialScenes_; // snapshot of JSON-loaded scenes
    std::unordered_map<int, std::string> keyMap; // GLFW_KEY_* to scene name
    std::unique_ptr<DrawItem> duplicate_item{ nullptr };


    std::vector<std::string> animal_name;
    //std::string backKey = "MainMenu";
    //std::string currentScene;

    AppState app{ AppState::Menu };                              // current mode
    std::string backKey;                                         // ESC destination (from JSON)
    bool requestedQuit{ false };                                  // quit flag
    std::string currentScene;
    std::string scenesFilePath; // store path

    ECS& ecs_;
    //std::unordered_map<std::string, Camera2D> cameras_; // one camera per scene

    std::function<void(const std::string&)> onSceneChanged;
   
    void writeAllComponentsToJSONArray(EntityID id,rapidjson::Value& overridesObj,rapidjson::Document::AllocatorType& alloc);

    bool scene_changed_since_click{ false };

    AnimationSystem animationSystem_;


};
