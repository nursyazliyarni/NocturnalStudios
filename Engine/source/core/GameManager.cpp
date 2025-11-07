/*!------------------------------------------------------------------------------------------------
    File: GameManager.cpp
    Project: NocturnalStudio

    Implements TextureStore and GameManager: JSON scene loading, click handling, rendering, and
    (optional) ECS sprite animation draw path.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "core/GameManager.h"

#include "stb_image.h"
#include <rapidjson/document.h>

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <ctime>
#include <cstdlib>
#include <cmath>

#include <GLFW/glfw3.h>

#include "ECS/ECS.h"
#include "ECS/ECS_Component.h"
#include "Animation/SpriteAnimationComponent.h"
#include "graphics/DebugDraw.h"
#include "physics/AABB.h"

#include "scene/Prefab.h" // Prefab_GetSceneName (if you need it later)

// ---------- TextureStore ----------
static GLuint loadGLTexture(const std::string& path) {
    int w, h, n;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 0);
    if (!data) return 0;

    GLenum fmt = (n == 4) ? GL_RGBA : (n == 3) ? GL_RGB : (n == 1) ? GL_RED : GL_RG;

    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    return id;
}

GLuint TextureStore::get(const std::string& path) {
    auto it = cache.find(path);
    if (it != cache.end()) return it->second;
    GLuint id = loadGLTexture(path);
    cache[path] = id;
    return id;
}

void TextureStore::clear() {
    for (auto& kv : cache) if (kv.second) glDeleteTextures(1, &kv.second);
    cache.clear();
}

// ---------- tiny file/JSON helpers ----------
static bool readFile(const char* p, std::string& out) {
    std::ifstream f(p);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

static std::string toAssetsPath(const std::string& p) {
    if (p.rfind("assets/", 0) == 0) return p;
    return "assets/" + p; // allow "textures/..." in JSON
}

struct PrefabDef { float w = 0.2f, h = 0.2f, angle = 0.0f; };

// ---------- GameManager ----------
bool GameManager::load(const char* prefabsPath, const char* scenesPath, TextureStore& tex) {
    app = AppState::Menu;      // lock number keys until Play
    requestedQuit = false;

    // 1) read prefab defaults (w,h,angle)
    std::unordered_map<std::string, PrefabDef> prefabDefaults;
    {
        std::string s;
        if (readFile(prefabsPath, s)) {
            rapidjson::Document d;
            d.Parse(s.c_str());
            if (!d.HasParseError() && d.IsObject() && d.HasMember("prefabs") && d["prefabs"].IsObject()) {
                const auto& pf = d["prefabs"];
                for (auto it = pf.MemberBegin(); it != pf.MemberEnd(); ++it) {
                    PrefabDef def;
                    const auto& val = it->value;
                    if (val.HasMember("defaults") && val["defaults"].IsObject()) {
                        const auto& jd = val["defaults"];
                        if (jd.HasMember("w")     && jd["w"].IsNumber())     def.w     = jd["w"].GetFloat();
                        if (jd.HasMember("h")     && jd["h"].IsNumber())     def.h     = jd["h"].GetFloat();
                        if (jd.HasMember("angle") && jd["angle"].IsNumber()) def.angle = jd["angle"].GetFloat();
                    }
                    prefabDefaults[it->name.GetString()] = def;
                }
            }
        }
    }

    // 2) read scenes
    std::string s;
    if (!readFile(scenesPath, s)) return false;
    rapidjson::Document d;
    d.Parse(s.c_str());
    if (d.HasParseError() || !d.IsObject()) return false;

    // states.initial + keymap + ESC backKey
    if (d.HasMember("states") && d["states"].IsObject()) {
        const auto& st = d["states"];
        if (st.HasMember("initial") && st["initial"].IsString()) currentScene = st["initial"].GetString();
        if (st.HasMember("keymap") && st["keymap"].IsObject()) {
            const auto& km = st["keymap"];
            auto mapKey = [&](const char* keyStr, int glfwKey) {
                if (km.HasMember(keyStr) && km[keyStr].IsString()) keyMap[glfwKey] = km[keyStr].GetString();
            };
            mapKey("1", GLFW_KEY_1);
            mapKey("2", GLFW_KEY_2);
            mapKey("3", GLFW_KEY_3);
            mapKey("4", GLFW_KEY_4);
            if (km.HasMember("ESC") && km["ESC"].IsString()) backKey = km["ESC"].GetString();
        }
    }

    // scenes{}
    if (!d.HasMember("scenes") || !d["scenes"].IsObject()) return false;
    const auto& jsScenes = d["scenes"];

    for (auto it = jsScenes.MemberBegin(); it != jsScenes.MemberEnd(); ++it) {
        const char* sceneName = it->name.GetString();
        const auto& jsS = it->value;
        if (!jsS.IsObject()) continue;

        Scene sc;

        // background
        if (jsS.HasMember("background") && jsS["background"].IsString()) {
            std::string bg = toAssetsPath(jsS["background"].GetString());
            sc.background = tex.get(bg);
        }

        // entities
        if (jsS.HasMember("entities") && jsS["entities"].IsArray()) {
            for (const auto& e : jsS["entities"].GetArray()) {
                if (!e.IsObject()) continue;

                PrefabDef def;
                if (e.HasMember("prefab") && e["prefab"].IsString()) {
                    auto itp = prefabDefaults.find(e["prefab"].GetString());
                    if (itp != prefabDefaults.end()) def = itp->second;
                }

                DrawItem di{};
                di.w = def.w; di.h = def.h; di.angle = def.angle;

                if (e.HasMember("overrides") && e["overrides"].IsObject()) {
                    const auto& ov = e["overrides"];
                    if (ov.HasMember("x")      && ov["x"].IsNumber())      di.x = ov["x"].GetFloat();
                    if (ov.HasMember("y")      && ov["y"].IsNumber())      di.y = ov["y"].GetFloat();
                    if (ov.HasMember("w")      && ov["w"].IsNumber())      di.w = ov["w"].GetFloat();
                    if (ov.HasMember("h")      && ov["h"].IsNumber())      di.h = ov["h"].GetFloat();
                    if (ov.HasMember("angle")  && ov["angle"].IsNumber())  di.angle = ov["angle"].GetFloat();
                    if (ov.HasMember("health_bar") && ov["health_bar"].IsNumber()) di.health_bar = ov["health_bar"].GetFloat();
                    if (ov.HasMember("hunger_bar") && ov["hunger_bar"].IsNumber()) di.hunger_bar = ov["hunger_bar"].GetFloat();
                    if (ov.HasMember("sanity_bar") && ov["sanity_bar"].IsNumber()) di.sanity_bar = ov["sanity_bar"].GetFloat();
                    if (ov.HasMember("sprite_path") && ov["sprite_path"].IsString()) {
                        std::string sp = toAssetsPath(ov["sprite_path"].GetString());
                        di.tex = tex.get(sp);
                    }
                }

                if (e.HasMember("name") && e["name"].IsString())
                    di.name = e["name"].GetString();

                sc.items.push_back(di);

                // clickable region for start/quit/goto
                ClickRegion cr{};
                cr.x = di.x; cr.y = di.y; cr.w = di.w; cr.h = di.h; cr.name = di.name;
                if (e.HasMember("goto")  && e["goto"].IsString()) cr.gotoScene = e["goto"].GetString();
                if (e.HasMember("quit")  && e["quit"].IsBool())   cr.quit      = e["quit"].GetBool();
                if (e.HasMember("start") && e["start"].IsBool())  cr.start     = e["start"].GetBool();

                if (cr.quit || cr.start || !cr.gotoScene.empty())
                    sc.clicks.push_back(cr);
            }
        }

        scenes[sceneName] = std::move(sc);
    }

    // TEMP: example ECS entity with animation 
    {
        EntityID flamingo = ecs_.createEntity();
        ecs_.addComponent<Transform>(flamingo, Transform{});
        ecs_.addComponent<ImageRenderer>(flamingo, ImageRenderer{});
        ecs_.addComponent<SpriteAnimationComponent>(flamingo, SpriteAnimationComponent{});

        if (auto* t = ecs_.getComponent<Transform>(flamingo)) {
            t->position = { 0.5f, 0.5f };
            t->scale    = { 0.25f, 0.25f };
        }
        if (auto* img = ecs_.getComponent<ImageRenderer>(flamingo)) {
            img->isVisible = true;
            img->textureID = tex.get("assets/textures/flamingo_spritesheet.png");
        }
        if (auto* anim = ecs_.getComponent<SpriteAnimationComponent>(flamingo)) {
            SpriteAnimationComponent::Animation idle;
            idle.frames    = 5;
            idle.frameTime = 0.12f;
            idle.loop      = true;
            anim->animations["idle"] = idle;
            anim->currentAnim  = "idle";
            anim->playing      = true;
            anim->currentFrame = 0;
            anim->sheetColumns = 5;
            anim->sheetRows    = 1;
        }
    }

    if (currentScene.empty() && !scenes.empty()) currentScene = scenes.begin()->first;
    return true;
}

void GameManager::requestScene(const std::string& name) {
    if (!scenes.count(name)) return;
    currentScene = name;
    if (onSceneChanged) onSceneChanged(currentScene);
}

bool GameManager::switchToSceneForKey(int glfwKey) {
    if (app != AppState::Playing) return false; // locked until start
    auto it = keyMap.find(glfwKey);
    if (it == keyMap.end()) return false;
    requestScene(it->second);
    return true;
}

void GameManager::handleKeyPress(int key) {
    if (key == GLFW_KEY_ESCAPE) {   // ESC always goes back to menu
        requestScene(backKey);
        goToMenu();
        return;
    }
    if (app == AppState::Menu) return; // ignore number keys in Menu

    auto it = keyMap.find(key);
    if (it != keyMap.end()) requestScene(it->second);
}

void GameManager::render(Renderer& r, int w, int h) {
    auto it = scenes.find(currentScene);
    if (it == scenes.end()) return;
    const Scene& sc = it->second;

    // Background
    if (it->first == "MainMenu") {
        // your special full-screen shader path for menu
        r.special_background_render(sc.background, "bgTexture", 1);
    } else {
        // standard background path
        r.background_render(sc.background, "bg_texture", 0);

        // Optionally also draw a textured quad that respects camera:
        float bx = 0.0f, by = 0.0f, bw = float(w), bh = float(h);
        Cam2D_Apply(currentScene, bx, by, bw, bh, w, h);
        r.texturedTri(sc.background, bx, by, bw, bh);
    }

    //  Draw ECS entities first (source of truth if same name exists) 
    if (!ecs_.getAllEntities().empty()) {
        constexpr EntityID MAX_SCAN = 5000;
        for (EntityID e = 0; e < MAX_SCAN; ++e) {
            auto* t   = ecs_.getComponent<Transform>(e);
            auto* img = ecs_.getComponent<ImageRenderer>(e);
            if (!t || !img) continue;
            if (!img->isVisible) continue;
            if (img->textureID <= 0) continue;

            float px = t->position.x() * w;
            float py = t->position.y() * h;
            float pw = t->scale.x()    * w;
            float ph = t->scale.y()    * h;

            Cam2D_Apply(currentScene, px, py, pw, ph, w, h);

            if (auto* anim = ecs_.getComponent<SpriteAnimationComponent>(e)) {
                auto itAnim = anim->animations.find(anim->currentAnim);
                if (itAnim != anim->animations.end()) {
                    int frameIndex    = anim->currentFrame;
                    int sheetColumns  = anim->sheetColumns > 0 ? anim->sheetColumns : 1;
                    int sheetRows     = anim->sheetRows    > 0 ? anim->sheetRows    : 1;

                    float uPerFrame = 1.0f / float(sheetColumns);
                    float vPerFrame = 1.0f / float(sheetRows);

                    int col = frameIndex % sheetColumns;
                    int row = frameIndex / sheetColumns;

                    float u0 = col * uPerFrame;
                    float v0 = row * vPerFrame;
                    float u1 = u0 + uPerFrame;
                    float v1 = v0 + vPerFrame;

                    r.drawSpriteUV(img->textureID, px, py, pw, ph, u0, v0, u1, v1);
                } else {
                    r.texturedTri(img->textureID, px, py, pw, ph);
                }
            } else {
                r.texturedTri(img->textureID, px, py, pw, ph);
            }

            // Optional debug AABB
            if (Debug::Drawer::Instance().IsActive()) {
                AABB collider(Vector2D(px, py), Vector2D(pw, ph));
                Debug::Drawer::Instance().AddRect(
                    collider.pos.x(), collider.pos.y(),
                    collider.size.x(), collider.size.y(),
                    0xFFFF00FF // yellow
                );
            }
        }
    }

    //  Draw JSON items only if there is no ECS entity with same name 
    for (const auto& di : sc.items) {
        if (!di.name.empty()) {
            EntityID eid = ecs_.findEntityByName(di.name);
            if (eid != INVALID_ENTITY_ID) continue; // avoid double-draw
        }
        if (di.tex == 0) continue;

        float px = di.x * w, py = di.y * h, pw = di.w * w, ph = di.h * h;

        if (currentScene != "MainMenu") {
            Cam2D_Apply(currentScene, px, py, pw, ph, w, h);
        }

        r.texturedTri(di.tex, px, py, pw, ph);

        if (Debug::Drawer::Instance().IsActive()) {
            AABB collider(Vector2D(px, py), Vector2D(pw, ph));
            Debug::Drawer::Instance().AddRect(
                collider.pos.x(), collider.pos.y(),
                collider.size.x(), collider.size.y(),
                0xFFFF00FF
            );
        }
    }
}

bool GameManager::handleClick(float nx, float ny) {
    auto it = scenes.find(currentScene);
    if (it == scenes.end()) return false;
    const Scene& sc = it->second;

    for (const auto& cr : sc.clicks) {
        bool hit = (nx >= cr.x && nx <= cr.x + cr.w &&
                    ny >= cr.y && ny <= cr.y + cr.h);

        logger.log(std::string("hit-test ")
            + (cr.name.empty() ? "<noname>" : cr.name)
            + (hit ? " 1" : " 0"));

        if (!hit) continue;

        logger.log("Click hit: start=" + std::to_string(cr.start) +
                   " quit=" + std::to_string(cr.quit) +
                   " goto='" + cr.gotoScene + "'");

        // Quit works in any state
        if (cr.quit) {
            logger.log("[Quit] click region flagged quit=true");
            if (quitCallback) quitCallback();
            else requestedQuit = true;
            return true;
        }

        // Menu: only start:true does anything
        if (app == AppState::Menu) {
            if (cr.start) {
                startGame();
                if (!cr.gotoScene.empty()) requestScene(cr.gotoScene);
                else if (auto it1 = keyMap.find(GLFW_KEY_1); it1 != keyMap.end())
                    requestScene(it1->second);
            }
            return true;
        }

        // Playing: allow goto
        if (!cr.gotoScene.empty()) {
            requestScene(cr.gotoScene);
        }
        return true;
    }
    return false;
}

void GameManager::startGame() { app = AppState::Playing; }
void GameManager::goToMenu()  { app = AppState::Menu;    }

bool GameManager::popQuitRequested() {
    bool q = requestedQuit;
    requestedQuit = false;
    return q;
}

Scene* GameManager::getCurrentScene() {
    auto it = scenes.find(currentScene);
    if (it != scenes.end()) return &it->second;
    return nullptr;
}

void GameManager::update(float dt, Renderer& renderer, TextureStore& tex) {
    // fixed‐step anim update (your AnimationSystem drives frames)
    const float fixedDt = 1.0f / 60.0f;
    animationSystem_.Update(fixedDt, ecs_, renderer, tex);
}
