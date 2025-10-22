/*!------------------------------------------------------------------------------------------------
    File: Application.cpp
    Project: NocturnalStudio

    This file contains the main application loop that initializes the window creation, OpenGL initialization,
    sprite rendering, input handling, drag and drop interations, frame updates, ImGui systems.

    Primary Author:

        - Brief

    Primary Author:
        Rishika Bimal Attodi
        - Implemented main loop scaffolding: window creation, OpenGL setup,
        sprite batching

    Secondary Author:
        Yu Xuan
        -  Background

    Secondary Authod
        Xiang Yang
        - FBO

    Secondary Author:
        Chang Qi Xuan Celest (35%)
        - Integrated drag and drop functionality for sprites.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
// init(), run(), shutdown()
#include "core/Application.h"
#include "core/Config.h"
#include <GLFW/glfw3.h>            // glViewport/glClearColor/glClear
#include <cmath>                   // cosf, sinf
#include <glm/gtc/constants.hpp>   // glm::pi<float>()
#include <chrono>
#include "math/Vector.h"
#include "core/ImGUIManager.h"
#include "./scene/Prefab.h"
#include "physics/AABB.h"

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <unordered_map>
#include <cstdio>

#include <array>
#include <vector>
#include <string>
#include <stdexcept>


#include <filesystem> 
#include <cctype>   // std::toupper
#include <unordered_set>

#include <cmath>


namespace {
    constexpr float TWO_PI = 2.0f * glm::pi<float>();
    constexpr int   CIRCLE_SEGMENTS = 64;
    using Clock = std::chrono::high_resolution_clock;
}

static inline float clamp01f(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }


static std::string FindFileUpwards(const std::string& rel) {
    namespace fs = std::filesystem;
    fs::path p = fs::current_path();
    for (int i = 0; i < 8; ++i) {     // climb at most 8 parent dirs
        fs::path candidate = p / rel;
        if (fs::exists(candidate))
            return candidate.string();
        if (p.has_parent_path()) p = p.parent_path(); else break;
    }
    // last resort: return rel so you still get a clear throw from fopen
    return rel;
}

//  Scene data read from assets/levels/zoo.json 
struct ItemDef {
    std::string name;
    std::string file;
};

struct SceneDef {
    int id{};
    std::string animal;
    std::string background;
    std::string sprite;
    std::string happy;
    float spawn[4]{};
    std::unordered_map<std::string, std::array<float, 4>> itemSpawns;
};

struct ZooDef {
    std::string menuBg;
    std::vector<ItemDef> items;
    std::vector<SceneDef> scenes;
};

static ZooDef gZoo;
static std::unordered_map<std::string, size_t> gTexIndexByTag; // e.g. "menu_bg","flamingo","flamingo_bg","drink"
size_t gCurrentBgTexSlot = 0; // used by ImGuiManager when drawing the background

static rapidjson::Document LoadJsonFile(const std::string& path) {
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) throw std::runtime_error("Could not open JSON: " + path);
    char buf[4096];
    rapidjson::FileReadStream is(fp, buf, sizeof(buf));
    rapidjson::Document d;
    d.ParseStream(is);
    fclose(fp);
    if (d.HasParseError()) throw std::runtime_error("Parse error in: " + path);
    return d;
}

static void LoadZoo(const std::string& path) {
    auto d = LoadJsonFile(path);
    if (d.HasMember("menu") && d["menu"].HasMember("background"))
        gZoo.menuBg = d["menu"]["background"].GetString();

    if (d.HasMember("items") && d["items"].IsArray()) {
        for (auto& it : d["items"].GetArray()) {
            ItemDef def;
            def.name = it["name"].GetString();
            def.file = it["file"].GetString();
            gZoo.items.push_back(def);
        }
    }

    if (d.HasMember("scenes") && d["scenes"].IsArray()) {
        for (auto& s : d["scenes"].GetArray()) {
            SceneDef sc;
            sc.id = s["id"].GetInt();
            sc.animal = s["animal"].GetString();
            sc.background = s["background"].GetString();
            sc.sprite = s["sprite"].GetString();
            sc.happy = s["happy"].IsNull() ? "" : s["happy"].GetString();
            for (int i = 0;i < 4;++i) sc.spawn[i] = s["spawn"][i].GetFloat();
            for (auto const& it : gZoo.items) {
                auto& isp = s["item_spawns"][it.name.c_str()];
                sc.itemSpawns[it.name] = { isp[0].GetFloat(), isp[1].GetFloat(), isp[2].GetFloat(), isp[3].GetFloat() };
            }
            gZoo.scenes.push_back(std::move(sc));
        }
    }
}



bool isDragging = false; //is a sprite being dragged
int draggedSpriteIndex = -1; //index into sprite_vector
int duplicateIndex = -1; //where clone is in the vector
Vector2D dragOffset(0.0f, 0.0f); //mouse offset from top-left of sprite

//Application::Application() = default;
Application::Application() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //enable memory leak check at program exit
}

int Application::run() {
    // 1) read JSON (with safe defaults)
    //const WindowConfig c = Config::LoadWindowConfig("assets/config.json");
    const GameConfig c = loadConfig("assets/config.json");

    // 2) create the window
    WindowDesc wd;
    wd.width = c.window.width;
    wd.height = c.window.height;
    wd.title = c.window.title;
    wd.vsync = c.window.vsync;

    if (!m_window.create(wd)) return -1;

    // Prime the window so there�s no white flash before the first real frame
    glViewport(0, 0, wd.width, wd.height);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    m_window.swap();


    // set up a simple 2D projection (pixel coords, origin = top-left) for the OS window
    int currW = wd.width, currH = wd.height;
    glfwGetFramebufferSize(m_window.handle(), &currW, &currH);
    glViewport(0, 0, currW, currH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, currW, currH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // sensible 2D state for the OS window (ImGui draws here later)
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    //Register GameObject Components
    RegisterAllComponents();

    // Initialize ImGui Manager
    // NOTE: ImGuiManager owns the Viewport pass; it will call Renderer to draw bg + shapes into an FBO.
    //       We just pass references here. No drawing is done in this file anymore.

 //   std::vector<char const*> vs{
 //   "assets/textures/DGPEN_LOGO.png",   // [0] background ONLY
 //   "assets/textures/drink.png",       // [1]
 //   "assets/textures/cupcake.png",       // [2]
 //   "assets/textures/placeholder.png",  // [3]
 //   "assets/textures/placeholder2.png",  // [4]
    //"assets/textures/catate.png",    // [5]
    //"assets/textures/catate2.png"   // [6]
 //   };

 //   Texture  txt_obj(vs);

        // load level definition
    LoadZoo(FindFileUpwards("assets/levels/zoo_level.json"));


    // build texture list dynamically
    std::vector<std::string> texPaths;
    gTexIndexByTag.clear();

    auto pushTex = [&](const std::string& tag, const std::string& file) {
        texPaths.push_back("assets/textures/" + file);
        gTexIndexByTag[tag] = texPaths.size() - 1;
        };

    // menu background
    pushTex("menu_bg", gZoo.menuBg);

    // items (drink/food/toy)
    for (auto const& it : gZoo.items) pushTex(it.name, it.file);

    // scenes: per animal background + sprite (+ happy if present)
    for (auto const& sc : gZoo.scenes) {
        pushTex(sc.animal + "_bg", sc.background);
        pushTex(sc.animal, sc.sprite);
        if (!sc.happy.empty()) pushTex(sc.animal + "_happy", sc.happy);
    }

    // create Texture from paths
    std::vector<char const*> vs;
    vs.reserve(texPaths.size());
    for (auto& s : texPaths) vs.push_back(s.c_str());

    Texture  txt_obj(vs);


    Renderer bg_render("assets/shaders/background.vert", "assets/shaders/background.frag");

    //  Load sprite prefabs (sizes & draggable flags) 
    PrefabManager prefab_man;  // create here so switchScene can use it
    prefab_man.loadSpriteArchetypes("assets/prefabs/prefabSprites.json", gTexIndexByTag);


    //build a set of draggable texture IDs (from prefab)
    std::unordered_set<int> draggableTexIds;
    {
        prefab_man.forEachSprite([&](const std::string&, const SpriteArchetype& a) {
            if (a.draggable) draggableTexIds.insert(a.texId);
            });
    }




    //  keep aspect if only one dimension is provided (w or h)
    auto fillAspectFromTex = [&](float& w, float& h, int texId) {
        int tw = txt_obj.get_texture_width((size_t)texId);
        int th = txt_obj.get_texture_height((size_t)texId);
        if (tw <= 0 || th <= 0) return;
        float aspect = (float)tw / (float)th; // width / height
        if (w > 0.f && h <= 0.f)      h = w / aspect;  // user gave width -> derive height
        else if (h > 0.f && w <= 0.f) w = h * aspect;  // user gave height -> derive width
        // if both > 0, respect the forced rectangle (may stretch by design)
        };


    auto switchScene = [&](int sceneId) {
        auto& sprites = bg_render.get_sprite_vector();
        sprites.clear();

        if (sceneId == 0) {
            gCurrentBgTexSlot = gTexIndexByTag.at("menu_bg");
            return; // menu: no sprites
        }

        // find scene
        const SceneDef* sc = nullptr;
        for (auto& s : gZoo.scenes) if (s.id == sceneId) { sc = &s; break; }
        if (!sc) { gCurrentBgTexSlot = gTexIndexByTag.at("menu_bg"); return; }

        // set background slot
        gCurrentBgTexSlot = gTexIndexByTag.at(sc->animal + "_bg");

        // add the animal first (index 0 in scene)
        {
            //  prefab keys are “Flamingo”, “Horse”, …
            {
                // TitleCase helper local to switchScene
                auto Title = [](std::string s) { if (!s.empty()) s[0] = (char)std::toupper((unsigned char)s[0]); return s; };

                int aniIdx = (int)gTexIndexByTag.at(sc->animal);

                float ax = sc->spawn[0], ay = sc->spawn[1];
                float aw = sc->spawn[2], ah = sc->spawn[3];

                bool wFromPrefab = false, hFromPrefab = false;
                if (const auto* arch = prefab_man.getSprite(Title(sc->animal))) {
                    if (arch->w > 0.f) { aw = arch->w; wFromPrefab = true; }
                    if (arch->h > 0.f) { ah = arch->h; hFromPrefab = true; }
                }
                // If only one side provided, zero the other so aspect can be derived
                if (wFromPrefab && !hFromPrefab) ah = 0.f;
                if (hFromPrefab && !wFromPrefab) aw = 0.f;

                fillAspectFromTex(aw, ah, aniIdx);
                sprites.push_back({ aniIdx, ax, ay, aw, ah, 0.0f });
            }


        }

        // then the three items (indices 1..3 in scene), override w/h if prefab provides
        for (auto const& it : gZoo.items) {
            const auto& rect = sc->itemSpawns.at(it.name);
            int id = (int)gTexIndexByTag.at(it.name);
            float ix = rect[0], iy = rect[1], iw = rect[2], ih = rect[3];

            auto Title = [](std::string s) { if (!s.empty()) s[0] = (char)std::toupper((unsigned char)s[0]); return s; };

            bool wFromPrefab = false, hFromPrefab = false;
            if (const auto* arch = prefab_man.getSprite(Title(it.name))) {
                if (arch->w > 0.f) { iw = arch->w; wFromPrefab = true; }
                if (arch->h > 0.f) { ih = arch->h; hFromPrefab = true; }
            }
            // match the “one side only” rule so aspect is preserved
            if (wFromPrefab && !hFromPrefab) ih = 0.f;
            if (hFromPrefab && !wFromPrefab) iw = 0.f;

            fillAspectFromTex(iw, ih, id);
            sprites.push_back({ id, ix, iy, iw, ih, 0.0f });
        }



        ////  EXTRA FOOD SPAWN (deterministic "random-ish", no <random>)
        //if (sceneId != 0) {
        //    auto itFood = std::find_if(gZoo.items.begin(), gZoo.items.end(),
        //        [](const ItemDef& d) { return d.name == "food"; });

        //    if (itFood != gZoo.items.end()) {
        //        auto fs = sc->itemSpawns.find("food"); // guard: scene has a 'food' rect
        //        if (fs != sc->itemSpawns.end()) {
        //            const auto& r = fs->second; // [x,y,w,h] area where food can appear
        //            float rx = r[0], ry = r[1], rw = r[2], rh = r[3];

        //            // prefab size override if present
        //            float fw = rw, fh = rh;
        //            if (const auto* arch = prefab_man.getSprite("Food")) {
        //                if (arch->w > 0.f) fw = arch->w;
        //                if (arch->h > 0.f) fh = arch->h;
        //            }

        //            auto clamp01 = [](float v) { return std::clamp(v, 0.0f, 1.0f); };
        //            int foodTex = static_cast<int>(gTexIndexByTag.at("food"));

        //            // tiny hash -> [0,1) using only sinf/floorf (deterministic)
        //            auto hash01 = [](int x) -> float {
        //                float s = std::sinf(x * 12.9898f) * 43758.5453f;
        //                return s - std::floor(s);
        //                };

        //            const int copies = 6;
        //            const int base = sceneId * 10007;

        //            for (int i = 0; i < copies; ++i) {
        //                float u = hash01(base + i * 2 + 1);
        //                float v = hash01(base + i * 2 + 2);

        //                // primary placement: keep whole sprite within rect
        //                float availW = rw - fw;
        //                float availH = rh - fh;

        //                float px, py;
        //                if (availW > 0.0f || availH > 0.0f) {
        //                    px = clamp01(rx + std::max(0.0f, availW) * u);
        //                    py = clamp01(ry + std::max(0.0f, availH) * v);
        //                }
        //                else {
        //                    // fallback jitter when rect == sprite size (or smaller)
        //                    float jx = (u - 0.5f) * fw * 0.6f;
        //                    float jy = (v - 0.5f) * fh * 0.6f;
        //                    px = clamp01(rx + jx);
        //                    py = clamp01(ry + jy);
        //                }

        //                sprites.push_back({ foodTex, px, py, fw, fh, 0.0f });
        //            }
        //        }
        //    }
        //}
        ////  END EXTRA FOOD SPAWN im ded


        //  EXTRA FOOD SPAWN ACROSS THE VIEWPORT (deterministic; no <random>)
        {
            int foodTex = static_cast<int>(gTexIndexByTag.at("food"));

            float fw = 0.15f, fh = 0.22f;
            bool wFromPrefab = false, hFromPrefab = false;
            if (const auto* arch = prefab_man.getSprite("Food")) {
                if (arch->w > 0.f) { fw = arch->w; wFromPrefab = true; }
                if (arch->h > 0.f) { fh = arch->h; hFromPrefab = true; }
            }

            // keep aspect if only one side provided by prefab
            if (wFromPrefab && !hFromPrefab) fh = 0.f;
            if (hFromPrefab && !wFromPrefab) fw = 0.f;
            fillAspectFromTex(fw, fh, foodTex);

            auto hash01 = [](int x)->float { float s = std::sinf(x * 12.9898f) * 43758.5453f; return s - std::floor(s); };
            auto clampRange = [](float v, float lo, float hi) { return std::max(lo, std::min(v, hi)); };

            const int copies = 8;
            const int base = sceneId * 10007 + 42;

            for (int i = 0; i < copies; ++i) {
                float u = hash01(base + i * 2 + 1);
                float v = hash01(base + i * 2 + 2);
                float px = clampRange(u * (1.0f - fw), 0.0f, 1.0f - fw);
                float py = clampRange(v * (1.0f - fh), 0.0f, 1.0f - fh);
                sprites.push_back({ foodTex, px, py, fw, fh, 0.0f });
            }
        }




        };

    // now we can set the initial scene (textures and renderer exist)
    switchScene(c.scene_id);



    Input      game_input_obj(m_window.handle());

    game_input_obj.initSpriteScales(txt_obj.get_totalTexture());

    game_input_obj.initSpriteScales(txt_obj.get_totalTexture());
    game_input_obj.initSpriteRotations(txt_obj.get_totalTexture());


    Physics    game_physics_obj;
    Game_Loop  game_loop_obj(m_window.handle());
    ImGuiManager imGuiManager(m_window, bg_render, txt_obj, game_input_obj, game_physics_obj);
    //PrefabManager prefab_man;

    // make Input�s scale array match the number of textures
    //game_input_obj.initSpriteScales(txt_obj.count());

    //to demonstrate cloning and rendering of gameobjects, this willl be moved into levelmanager
    // Register Food prefab (uses the already loaded "food" texture)
    //{
    //    auto it = gTexIndexByTag.find("food");
    //    if (it != gTexIndexByTag.end()) {
    //        SpriteArchetype food;
    //        food.texId = static_cast<int>(it->second);

    //        // optional: derive default size from JSON item_spawns["food"]
    //        // fallback size kept if JSON missing
    //        // (we’ll set size per scene during spawn anyway)
    //        prefab_man.registerSprite("Food", food);
    //    }
    //}

    // 4) simple loop
    while (!m_window.shouldClose()) {
        m_window.poll();

        // scene switching driven by Input system (A/B/C/D/M)
        {
            int sw = game_input_obj.consumeSceneSwitch();
            if (sw >= 0) {
                switchScene(sw);
            }
        }



        double dt = game_loop_obj.calculate_DT();          // our deltaTime
        double total_time = game_loop_obj.get_accum_GT();  // accumulated game time
        game_loop_obj.display_FPS(m_window.handle(), c.window.title, dt); // display FPS
        game_input_obj.update_mouse_pos(m_window.handle());
        game_input_obj.GET_mouse_delta();


        //---------------------DRAG AND DROP---------------------------
        //double mouseX, mouseY;
        //glfwGetCursorPos(m_window.handle(), &mouseX, &mouseY);
        //Vector2D mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));

        //auto& sprites = bg_render.get_sprite_vector();
        //std::vector<int> frontIdxs; //indices to draw on top

        //if (!isDragging) {
        //    if (glfwGetMouseButton(m_window.handle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        //        for (int i = 0; i < (int)sprites.size(); i++) {
        //            // in each scene: sprites[0] is the animal, sprites[1..3] are items (drink/food/toy)
        //            //if (i < 1 || i > 3) continue; // only the three items are draggable
        //            if (!draggableTexIds.count(sprites[i].idx)) continue; // prefab-driven draggability
        //            const auto& spr = sprites[i];

        //            float x = spr.x * currW;
        //            float y = spr.y * currH;
        //            float w = spr.w * currW;
        //            float h = spr.h * currH;

        //            if (isPointInAABB(mousePos, { x, y }, { w, h })) {
        //                //create duplicate and bring it to front
        //                sprites.push_back(spr);
        //                duplicateIndex = (int)sprites.size() - 1;
        //                draggedSpriteIndex = i;
        //                isDragging = true;
        //                dragOffset = mousePos - Vector2D(x, y);

        //                frontIdxs.push_back(sprites[duplicateIndex].idx); //ensure duplicate is drawn on top
        //                break;
        //            }
        //        }
        //    }
        //}
        //else { //isDragging
        //    if (glfwGetMouseButton(m_window.handle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        //        if (duplicateIndex >= 0 && duplicateIndex < (int)sprites.size()) {
        //            auto& dup = sprites[duplicateIndex];
        //            frontIdxs.push_back(sprites[duplicateIndex].idx);

        //            //check drop target
        //            for (int i = 0; i < (int)sprites.size(); ++i) {
        //                if (!draggableTexIds.count(sprites[i].idx)) continue; // prefab-driven draggability
        //                auto& target = sprites[i];
        //                //target is a cat
        //                                   // snap test: drop onto the ANIMAL (sprites[0])
        //                if (!sprites.empty()) {
        //                    auto& animal = sprites[0];
        //                    float tx = animal.x * currW;
        //                    float ty = animal.y * currH;
        //                    float tw = animal.w * currW;
        //                    float th = animal.h * currH;

        //                    if (isPointInAABB(Vector2D(dup.x * currW, dup.y * currH), { tx, ty }, { tw, th })) {
        //                        // if the current animal has a "_happy" variant, swap it
        //                        // find scene by matching background slot
        //                        const SceneDef* sc = nullptr;
        //                        // we can deduce scene by checking current bg slot vs known tags, but simpler:
        //                        // check all scenes for animal.id == this sprite's "animal"
        //                        // here we infer from the current gCurrentBgTexSlot in ImGui pass (background)
        //                        // instead, use the 1st sprite texture tag to find the animal name
        //                        // we don't have a reverse map -> keep it simple: try all scenes for a matching "animal" sprite index
        //                        for (auto& s : gZoo.scenes) {
        //                            size_t aniIdx = gTexIndexByTag[s.animal];
        //                            if ((int)aniIdx == animal.idx) { sc = &s; break; }
        //                        }
        //                        if (sc && !sc->happy.empty()) {
        //                            animal.idx = (int)gTexIndexByTag[sc->animal + "_happy"];
        //                        }
        //                    }
        //                }

        //            }

        //            // remove duplicate
        //            sprites.erase(sprites.begin() + duplicateIndex);
        //        }

        //        isDragging = false;
        //        draggedSpriteIndex = -1;
        //        duplicateIndex = -1;
        //        frontIdxs.clear();
        //    }
        //    else {
        //        // move duplicate with mouse
        //        auto& spr = sprites[duplicateIndex];
        //        float newX = mousePos.x() - dragOffset.x();
        //        float newY = mousePos.y() - dragOffset.y();
        //        spr.x = newX / currW;
        //        spr.y = newY / currH;

        //        frontIdxs.clear();
        //        frontIdxs.push_back(spr.idx); // always draw duplicate on top
        //    }
        //}

        // draw sprites: first behind pass (everything except front), then front pass
        //bg_render.drawAllSprites(txt_obj, frontIdxs, true, currW, currH);   // behind pass
        //bg_render.drawAllSprites(txt_obj, frontIdxs, false, currW, currH);  // front pass
        //---------------------END DRAG AND DROP---------------------------

        // handle resize: update viewport/projection if size changed (no callbacks)

        // handle OS-window resize: keep a sane viewport/projection for the UI backdrop
        int fbw, fbh;
        glfwGetFramebufferSize(m_window.handle(), &fbw, &fbh);
        if (fbw != currW || fbh != currH) {
            currW = fbw; currH = fbh;
            glViewport(0, 0, currW, currH);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, currW, currH, 0, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
        }

        // Clear the OS window behind the UI (the actual scene is drawn inside the Viewport FBO)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, currW, currH);
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        auto imGuiStart = Clock::now();
        // Build & render all ImGui (Viewport will render bg + shapes internally via Renderer)
       /* auto imGuiStart = Clock::now();*/
        // Build & render all ImGui (Viewport draws bg+sprites inside FBO)
        Gui::updateImGuiManager(dt, imGuiManager);
        auto imGuiEnd = Clock::now();
        double imGuiTime = std::chrono::duration<double>(imGuiEnd - imGuiStart).count();

        // === DRAG & DROP (viewport-normalized, runs AFTER ImGui so rect is valid) ===
        {
            double mouseX, mouseY;
            glfwGetCursorPos(m_window.handle(), &mouseX, &mouseY);

            // Viewport image rect in screen coords (set in showViewportWindowUI())
            ImVec2 vpPos = imGuiManager.getViewportImagePos();
            ImVec2 vpSize = imGuiManager.getViewportImageSize();
            float  vpX = vpPos.x, vpY = vpPos.y, vpW = vpSize.x, vpH = vpSize.y;

            // Mouse must be inside the viewport image
            bool mouseInViewport = (mouseX >= vpX && mouseX <= vpX + vpW &&
                mouseY >= vpY && mouseY <= vpY + vpH);

            // Mouse in viewport-normalized [0..1]
            float mxN = mouseInViewport ? float((mouseX - vpX) / vpW) : -1.f;
            float myN = mouseInViewport ? float((mouseY - vpY) / vpH) : -1.f;


            //  HOVER PICK (top-most sprite under mouse, draggable or not) 
            int hoveredIndex = -1;
            if (mouseInViewport) {
                // we already have: auto& sprites = bg_render.get_sprite_vector();
                const auto& spritesConst = bg_render.get_sprite_vector();
                for (int i = (int)spritesConst.size() - 1; i >= 0; --i) {
                    const auto& spr = spritesConst[i];
                    const float sx0 = spr.x, sy0 = spr.y;
                    const float sx1 = spr.x + spr.w, sy1 = spr.y + spr.h;
                    if (mxN >= sx0 && mxN <= sx1 && myN >= sy0 && myN <= sy1) {
                        hoveredIndex = i;
                        break;
                    }
                }
            }


            auto& sprites = bg_render.get_sprite_vector();
            std::vector<int> frontIdxs; // render-on-top list
            static Vector2D dragOffsetNorm(0.f, 0.f); // normalized offset inside sprite

            if (!isDragging) {
                if (mouseInViewport &&
                    glfwGetMouseButton(m_window.handle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

                    // top-most draggable hit: iterate back-to-front
                    for (int i = int(sprites.size()) - 1; i >= 0; --i) {
                        if (!draggableTexIds.count(sprites[i].idx)) continue;

                        const auto& spr = sprites[i];
                        float sx0 = spr.x, sy0 = spr.y;
                        float sx1 = spr.x + spr.w;
                        float sy1 = spr.y + spr.h;

                        if (mxN >= sx0 && mxN <= sx1 && myN >= sy0 && myN <= sy1) {
                            sprites.push_back(spr);                  // duplicate on top
                            duplicateIndex = int(sprites.size()) - 1;
                            draggedSpriteIndex = i;
                            isDragging = true;

                            dragOffsetNorm = Vector2D(mxN - spr.x, myN - spr.y);
                            frontIdxs.push_back(sprites[duplicateIndex].idx);
                            break;
                        }
                    }
                }
            }
            else {
                if (glfwGetMouseButton(m_window.handle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
                    if (duplicateIndex >= 0 && duplicateIndex < (int)sprites.size()) {
                        auto& dup = sprites[duplicateIndex];
                        frontIdxs.push_back(dup.idx);

                        // drop onto animal (index 0) → set _happy if present
                        if (!sprites.empty() && mouseInViewport) {
                            auto& animal = sprites[0];
                            float tx0 = animal.x, ty0 = animal.y;
                            float tx1 = animal.x + animal.w, ty1 = animal.y + animal.h;

                            if (dup.x >= tx0 && dup.x <= tx1 && dup.y >= ty0 && dup.y <= ty1) {
                                const SceneDef* sc = nullptr;
                                for (auto& s : gZoo.scenes) {
                                    size_t aniIdx = gTexIndexByTag[s.animal];
                                    if ((int)aniIdx == animal.idx) { sc = &s; break; }
                                }
                                if (sc && !sc->happy.empty()) {
                                    animal.idx = (int)gTexIndexByTag[sc->animal + "_happy"];
                                }
                            }
                        }

                        sprites.erase(sprites.begin() + duplicateIndex);
                    }

                    isDragging = false;
                    draggedSpriteIndex = -1;
                    duplicateIndex = -1;
                    frontIdxs.clear();
                }
                else {
                    // move duplicate with mouse
                    if (mouseInViewport && duplicateIndex >= 0 && duplicateIndex < (int)sprites.size()) {
                        auto& spr = sprites[duplicateIndex];

                        float newX = mxN - dragOffsetNorm.x();
                        float newY = myN - dragOffsetNorm.y();

                        // clamp so whole sprite stays visible
                        spr.x = std::min(std::max(0.0f, newX), 1.0f - spr.w);
                        spr.y = std::min(std::max(0.0f, newY), 1.0f - spr.h);

                        frontIdxs.clear();
                        frontIdxs.push_back(spr.idx); // draw on top while dragging
                    }
                }
            }

            // Always hand the list to ImGuiManager each frame (even if empty)
            imGuiManager.setFrontSpriteIdxs(frontIdxs);

            // time-based rotation on the hovered sprite (J/K toggles)
            /*game_physics_obj.update_sprite_rotation(bg_render, game_input_obj, dt, hoveredIndex);*/
            // time-based rotation on the hovered sprite (J/K toggles)


           // === ROTATION: toy-only (no hover) ===

// Resolve the TOY texture id once (safe if tag is missing) RISHIKA
            static int toyTexId = [] {
                auto it = gTexIndexByTag.find("toy");     // if your JSON uses "ball", change to "ball"
                return (it != gTexIndexByTag.end()) ? (int)it->second : -1;
                }();

            // Find the current toy *sprite index* in the renderer’s sprite list
            int toySpriteIndex = -1;
            if (toyTexId >= 0) {
                const auto& sprites = bg_render.get_sprite_vector();
                for (int i = (int)sprites.size() - 1; i >= 0; --i) {
                    if (sprites[i].idx == toyTexId) {     // this sprite uses the toy texture
                        toySpriteIndex = i;
                        break;
                    }
                }
            }

            // Call rotation update with our forced selection (only the toy rotates)
            game_physics_obj.update_sprite_rotation(bg_render, game_input_obj, dt, toySpriteIndex);



        }

        // === END DRAG & DROP ===



        imGuiManager.setSystemTimings(imGuiTime, 0.0);


        m_window.swap();
    }

    m_window.destroy();
    return 0;
}
