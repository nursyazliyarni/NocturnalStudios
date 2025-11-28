/*!------------------------------------------------------------------------------------------------
    File: GameManager.cpp
    Project: NocturnalStudio

    This file contains the implementation of the GameManager class


    Primary Author:
        Rishika Bimal Attodi %
        -State integration for the play and exit button and scene switching via keys

    Secondary Author:
        Nur Syazliyarni (5%)
        - Added function to get current scene for level editor

        Lau Xiang Yang (2%)
        - Implemented pause and resume functionality in GameManager

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/


#include "core/GameManager.h"
#include "stb_image.h"
#include <rapidjson/document.h>
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filewritestream.h"
#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>

#include "ECS/ECS_Component.h"
#include "ECS/ECS.h"

#include "graphics/Camera2D.h"
#include "graphics/DebugDraw.h"
#include "physics/AABB.h"
#include "Animation/SpriteAnimationComponent.h"

#include "core/Application.h"

#include <functional>

//  TextureStore 
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

std::string TextureStore::find(const GLuint id) const {
    for (auto& it:cache)
    {
        if (it.second == id) return it.first;
    }
    return"";
}

void TextureStore::clear() {
    for (auto& kv : cache) if (kv.second) glDeleteTextures(1, &kv.second);
    cache.clear();
}

//  tiny JSON helpers 
static bool readFile(const char* p, std::string& out) {
    std::ifstream f(p);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

std::unique_ptr<DrawItem>& GameManager::get_duplicate_item() {
    return duplicate_item;
}

static std::string toAssetsPath(const std::string& p) {
    if (p.rfind("assets/", 0) == 0) return p;
    return "assets/" + p; // scenes use "textures/.." to make "assets/textures/.."
}

struct PrefabDef { float w = 0.2f, h = 0.2f, angle = 0.0f; };

//  GameManager 
bool GameManager::saveScene( std::string scenename, const char* scenesJsonPath, TextureStore& tex) {
    if (scenename == "___currscene___")scenename = currentScene;

    logger.log("Attempting to save scene: " + scenename);

    auto sitr = scenes.find(scenename);
    if (sitr == scenes.end()) { 
        logger.log("Scene not found: " + scenename);
        return false; }

    Scene& scene = sitr->second;

    rapidjson::Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    if (scene.background) {
        rapidjson::Value bgVal;
        bgVal.SetString(tex.find(scene.background).c_str(), alloc);
        doc.AddMember("background", bgVal, alloc);
        
    }

    if (!scene.items.empty())
    {
        rapidjson::Value entityArray(rapidjson::kArrayType);

        for (DrawItem i: scene.items){
            rapidjson::Value entObj(rapidjson::kObjectType);
            if (i.item_type.empty()) //treat as a non prefab item
            {

            }
            else {
                //prefab has 3 things: prefab, name, overrides
                rapidjson::Value pregabval;
                pregabval.SetString(i.item_type.c_str(), alloc);
                entObj.AddMember("prefab", pregabval, alloc);
                
                rapidjson::Value nvs;
                nvs.SetString(i.name.c_str(), alloc);
                entObj.AddMember("name", nvs, alloc);
                //overrides
                rapidjson::Value overrides(rapidjson::kObjectType);
                if (ecs_.findEntityByName(i.name)!=INVALID_ENTITY_ID&&!ecs_.getEntityComponents(ecs_.findEntityByName(i.name)).empty())
                {
                    writeAllComponentsToJSON(ecs_.findEntityByName(i.name), overrides, alloc);
                }
                entObj.AddMember("overrides", overrides, alloc);
            }
            entityArray.PushBack(entObj, alloc);
        }
        doc.AddMember("entities", entityArray, alloc);
    }

    FILE* fp = fopen(scenesJsonPath, "wb");
    if (!fp) return false;

    char buffer[9920];
    rapidjson::FileWriteStream os(fp, buffer, sizeof(buffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    doc.Accept(writer);
    fclose(fp);
    return true;
}


bool GameManager::load(const char* prefabsPath, const char* scenesPath, TextureStore& tex) {

    app = AppState::Menu;   // lock number keys until Play is pressed
    requestedQuit = false;  // (optional) clear quit flag at load

    // Store paths for later reload
    scenesFilePath = scenesPath;

    //  1) read prefab defaults (w,h,angle) 
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
                        if (jd.HasMember("w") && jd["w"].IsNumber())     def.w = jd["w"].GetFloat();
                        if (jd.HasMember("h") && jd["h"].IsNumber())     def.h = jd["h"].GetFloat();
                        if (jd.HasMember("angle") && jd["angle"].IsNumber()) def.angle = jd["angle"].GetFloat();
                    }
                    prefabDefaults[it->name.GetString()] = def;
                }
            }
        }
    }

    // ---- 2) read scenes ----
    std::string s;
    if (!readFile(scenesPath, s)) return false;
    rapidjson::Document d;
    d.Parse(s.c_str());
    if (d.HasParseError() || !d.IsObject()) return false;

    // states
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

    // scenes
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
                DrawItem di{};
                if (e.HasMember("prefab") && e["prefab"].IsString()) {
                    auto itp = prefabDefaults.find(e["prefab"].GetString());
                    if (itp != prefabDefaults.end()) def = itp->second;
                    if (e.HasMember("prefab") && e["prefab"].IsString()) {
                        di.item_type = e["prefab"].GetString();  // always safe

                        auto itp = prefabDefaults.find(di.item_type);
                        if (itp != prefabDefaults.end()) {
                            def = itp->second;
                        }
                        else {
                            logger.log(std::string("[load] prefab not found in prefabs.json: ") + di.item_type);
                        }
                    }
                }

                di.w = def.w;
                di.h = def.h;
                di.angle = def.angle;

                if (e.HasMember("overrides") && e["overrides"].IsObject()) {
                    const auto& ov = e["overrides"];
                    if (ov.HasMember("x") && ov["x"].IsNumber())      di.x = ov["x"].GetFloat();
                    if (ov.HasMember("y") && ov["y"].IsNumber())      di.y = ov["y"].GetFloat();
                    if (ov.HasMember("w") && ov["w"].IsNumber())      di.w = ov["w"].GetFloat();
                    if (ov.HasMember("h") && ov["h"].IsNumber())      di.h = ov["h"].GetFloat();
                    if (ov.HasMember("angle") && ov["angle"].IsNumber()) di.angle = ov["angle"].GetFloat();
                    if (ov.HasMember("is_draggable") && ov["is_draggable"].IsBool()) di.is_draggable = ov["is_draggable"].GetBool();
                    if (ov.HasMember("is_dragging") && ov["is_dragging"].IsBool()) di.is_dragging = ov["is_dragging"].GetBool();

         

                    if (ov.HasMember("sprite_path") && ov["sprite_path"].IsString()) {
                        std::string sp = toAssetsPath(ov["sprite_path"].GetString());
                        di.tex = tex.get(sp);
                    }
                }

                if (e.HasMember("animal_properties") && e["animal_properties"].IsObject()) {
                    const auto& animal = e["animal_properties"];
                    if (animal.HasMember("health_bar") && animal["health_bar"].IsNumber()) di.health_bar = animal["health_bar"].GetFloat();
                    if (animal.HasMember("hunger_bar") && animal["hunger_bar"].IsNumber()) di.hunger_bar = animal["hunger_bar"].GetFloat();
                    if (animal.HasMember("sanity_bar") && animal["sanity_bar"].IsNumber()) di.sanity_bar = animal["sanity_bar"].GetFloat();

                    if (animal.HasMember("slower_decreasing_rate") && animal["slower_decreasing_rate"].IsNumber()) di.slower_decreasing_rate = animal["slower_decreasing_rate"].GetFloat();
                    if (animal.HasMember("normal_decreasing_rate") && animal["normal_decreasing_rate"].IsNumber()) di.normal_decreasing_rate = animal["normal_decreasing_rate"].GetFloat();
                    if (animal.HasMember("faster_decreasing_rate") && animal["faster_decreasing_rate"].IsNumber()) di.faster_decreasing_rate = animal["faster_decreasing_rate"].GetFloat();
                    if (animal.HasMember("penalty_decreasing_rate") && animal["penalty_decreasing_rate"].IsNumber()) di.penalty_decreasing_rate = animal["penalty_decreasing_rate"].GetFloat();
                    di.still_alive = true;
                }

                if (e.HasMember("name") && e["name"].IsString())
                    di.name = e["name"].GetString();


                sc.items.push_back(di);

                ClickRegion cr;
                cr.x = di.x; cr.y = di.y; cr.w = di.w; cr.h = di.h;
                cr.name = di.name;
                if (e.HasMember("goto") && e["goto"].IsString()) cr.gotoScene = e["goto"].GetString();
                if (e.HasMember("quit") && e["quit"].IsBool())   cr.quit = e["quit"].GetBool();
                if (e.HasMember("start") && e["start"].IsBool())  cr.start = e["start"].GetBool();

                // store the region if it does ANY action
                if (cr.quit || cr.start || !cr.gotoScene.empty())
                    sc.clicks.push_back(cr);


            }
        }

        scenes[sceneName] = std::move(sc);
    }
	//temp: add a flamingo entity with sprite animation to ECS
    {
        EntityID flamingo = ecs_.createEntity();

        ecs_.addComponent<Transform>(flamingo, Transform{});
        ecs_.addComponent<ImageRenderer>(flamingo, ImageRenderer{});
        ecs_.addComponent<SpriteAnimationComponent>(flamingo, SpriteAnimationComponent{});

        auto* t = ecs_.getComponent<Transform>(flamingo);
        auto* img = ecs_.getComponent<ImageRenderer>(flamingo);
        auto* anim = ecs_.getComponent<SpriteAnimationComponent>(flamingo);

        t->position = { 0.5f, 0.5f };
        t->scale = { 0.25f, 0.25f };

        img->isVisible = true;
        img->textureID = tex.get("assets/textures/flamingo_spritesheet.png");

        //Define an animation sequence
        SpriteAnimationComponent::Animation idle;
        idle.frames = 5;
        idle.frameTime = 0.12f;
        idle.loop = true;
        idle.spritePath = "assets/textures/flamingo_spritesheet.png";
        idle.sheetColumns = 5;
        idle.sheetRows = 1;

        anim->animations["idle"] = idle;
        anim->currentAnim = "idle";
        anim->playing = true;
        anim->currentFrame = 0;
        anim->sheetColumns = 5;
        anim->sheetRows = 1;
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
    if (app != AppState::Playing) return false;   // lock numbers unless Playing
    auto it = keyMap.find(glfwKey);
    if (it == keyMap.end()) return false;         // not mapped in JSON
    requestScene(it->second);
    return true;
}

void GameManager::handleKeyPress(int key) {
    // ESC always returns to menu
    if (key == GLFW_KEY_ESCAPE) {
        requestScene(backKey);
        goToMenu(); // relock number keys
        return;
    }

    // If we're still in Menu, ignore number keys 1..9 entirely
    if (app == AppState::Menu) {
        return;
    }

    // Playing: allow mapped numeric switches
    auto it = keyMap.find(key);
    if (it != keyMap.end()) {
        requestScene(it->second);
    }
}

void GameManager::render(Renderer& r, Input& input, GLFWwindow* window, int w, int h) {


    auto it = scenes.find(currentScene);
    if (it == scenes.end()) {  return; }
    const Scene& sc = it->second;

    // background (fullscreen) � NOTE uniform must be "bg_texture"
    if (it->first == "MainMenu") {
        r.special_background_render(sc.background, "bgTexture", 1);
    }

    else {
        r.background_render(sc.background, "bg_texture", 0);
        // Draw background as a regular textured quad and apply camera to it too
        float bx = 0.0f, by = 0.0f, bw = float(w), bh = float(h);
        Cam2D_Apply(currentScene, bx, by, bw, bh, w, h);
        r.texturedTri(sc.background, bx, by, bw, bh);
    }

    constexpr EntityID MAX_SCAN = 5000;
    for (EntityID e = 0; e < MAX_SCAN; ++e) {
        if (auto* t = ecs_.getComponent<Transform>(e)) {
            if (auto* img = ecs_.getComponent<ImageRenderer>(e)) {
                if (!img->isVisible) continue;
                if (img->textureID <= 0) continue;  // << guard: skip �white box� draws

                if (auto* anim = ecs_.getComponent<SpriteAnimationComponent>(e)) {
                    auto it = anim->animations.find(anim->currentAnim);
                    if (it != anim->animations.end()) {
                        const auto& def = it->second;

                        int frameIndex = anim->currentFrame;
                        int sheetColumns = anim->sheetColumns;
                        int sheetRows = anim->sheetRows;

                        float uPerFrame = 1.0f / float(sheetColumns);
                        float vPerFrame = 1.0f / float(sheetRows);

                        int col = frameIndex % sheetColumns;
                        int row = frameIndex / sheetColumns;

                        float u0 = col * uPerFrame;
                        float v0 = row * vPerFrame;
                        float u1 = u0 + uPerFrame;
                        float v1 = v0 + vPerFrame;

                        float px = t->position.x() * w;
                        float py = t->position.y() * h;
                        float pw = t->scale.x() * w;
                        float ph = t->scale.y() * h;

                        Cam2D_Apply(currentScene, px, py, pw, ph, w, h);
                        r.drawSpriteUV(img->textureID, px, py, pw, ph, u0, v0, u1, v1);
                    }
                }
            }
        }
    }

    // entities (normalized ? pixels)
   // entities (normalized → pixels) — draw JSON **only** if there is NO ECS entity by the same name
    for (const auto& di : sc.items) {
        // If an ECS entity with this name exists, we skip the JSON-drawn duplicate
        if (!di.name.empty()) {
            EntityID eid = ecs_.findEntityByName(di.name);
            if (eid != INVALID_ENTITY_ID) {
                continue; // ECS is the source of truth for this sprite
            }
        }

        if (di.tex == 0) continue;
        float px = di.x * w, py = di.y * h, pw = di.w * w, ph = di.h * h;

        if (di.name=="flamingo"||di.name=="seal"||di.name=="owl"||di.name=="horse") {
            if (di.still_alive) {
                r.rect(w / 2, h / 2 - 200, di.health_bar * 4, 25, 1, 0, 0);
                r.rect(w / 2, h / 2 - 150, di.hunger_bar * 4, 25, 0, 1, 0);
                r.rect(w / 2, h / 2 - 100, di.sanity_bar * 4, 25, 0, 0, 1);

                Cam2D_Apply(currentScene, px, py, pw, ph, w, h);
                r.texturedTri(di.tex, px, py, pw, ph);
            }
        }

        else {
            Cam2D_Apply(currentScene, px, py, pw, ph, w, h);
            r.texturedTri(di.tex, px, py, pw, ph);

            // --- COLLIDER VISUALIZATION ---
            if (di.hasCollider)
            {
                float colX = di.x * w + di.colOffsetX * w;
                float colY = di.y * h + di.colOffsetY * h;
                float colW = di.colW * w;
                float colH = di.colH * h;

                // convert collider location from center to top-left
                float cx = colX + colW * 0.5f;
                float cy = colY + colH * 0.5f;

                // green rectangle
                r.rect(cx, cy, colW, colH, 0.0f, 1.0f, 0.0f);
            }
            // --------------------------------
        }

        if (di.is_dragging && !duplicate_item) {
            duplicate_item = std::make_unique<DrawItem>(di);
        }

        if (duplicate_item) {
            r.texturedTri(duplicate_item->tex, duplicate_item->x * w, duplicate_item->y * h, duplicate_item->w * w, duplicate_item->h * h);
        }

        //if (!input.isLeftMousePressed(window)) {
        //    duplicate_item.reset(); //The unique_ptr releases ownership
        //}

        /*if (Debug::Drawer::Instance().IsActive()) {
            AABB collider(Vector2D(px, py), Vector2D(pw, ph));
            Debug::Drawer::Instance().AddRect(
                collider.pos.x(), collider.pos.y(),
                collider.size.x(), collider.size.y(),
                0xFFFF00FF
            );
        }*/
		// --- Debug Draw Collider Box ---
        if (Debug::Drawer::Instance().IsActive() && di.hasCollider)
        {
            float cx = di.x + di.colOffsetX;
            float cy = di.y + di.colOffsetY;

            float px = cx * w;
            float py = cy * h;

            float cw = di.colW * w;
            float ch = di.colH * h;

            Debug::Drawer::Instance().AddRect(
                px - cw * 0.5f,
                py - ch * 0.5f,
                cw,
                ch,
                0x00FF00FF //green box
            );
        }
    }
}




void GameManager::renderWithHUD(Renderer& r,
    Input& input,
    GLFWwindow* window,
    int w, int h,
    double dt,
    ResourceManager& resourceManager,
    bool showFPS)
{
    // 1) First, render the usual scene
    render(r, input, window, w, h);

    // 2) Now overlay HUD text into the same render target
    r.setTextProjection(w, h); // bottom-left origin for text shader

    // Cache fonts the first time we’re called
    static std::optional<ResourceID> s_fontTitle;
    static std::optional<ResourceID> s_fontFPS;

    if (!s_fontTitle) {
        s_fontTitle = resourceManager.findResource<Font>("NanumBrushScript-Regular");
    }
    if (!s_fontFPS) {
        s_fontFPS = resourceManager.findResource<Font>("PAPYRUS");
    }

    // --- Responsive scaling based on viewport size ---
    // Pretend our "reference" resolution is 1280x720
    const float refW = 1280.0f;
    const float refH = 720.0f;

    float sx = static_cast<float>(w) / refW;
    float sy = static_cast<float>(h) / refH;

    // Use the smaller scale so text doesn't overflow in one direction
    float textScale = std::min(sx, sy);

    // Clamp so it doesn’t get insanely small/huge
    if (textScale < 0.4f) textScale = 0.4f;
    if (textScale > 2.5f) textScale = 2.5f;

    // If viewport is super tiny, just skip HUD entirely
    if (w < 150 || h < 120) {
        return;
    }

    // --- Title (Font A) ---
    if (s_fontTitle) {
        const Font& f = resourceManager.getFont(*s_fontTitle);

        // Keep margins as percentage of viewport → stays nicely anchored
        float marginX = 0.02f * w;
        float marginTop = 0.08f * h;

        float titleX = marginX;
        float titleY = h - marginTop;

        r.drawText(
            f,
            "Nocturnal Studio Engine",
            titleX,
            titleY,
            textScale,
            glm::vec3(1.0f, 0.0f, 0.2f) // reddish
        );
    }

    // --- FPS (Font B) ---
    if (showFPS && s_fontFPS) {
        // If viewport height is small, we show FPS but tuck it closer
        bool compact = (h < 300);

        const Font& f2 = resourceManager.getFont(*s_fontFPS);

        static double accumTime = 0.0;
        static double fpsDisplay = 0.0;

        // avoid division by zero
        if (dt <= 0.0) dt = 1e-6;

        accumTime += dt;
        if (accumTime >= 1.0) {
            fpsDisplay = 1.0 / dt;
            accumTime = 0.0;
        }

        int fpsInt = static_cast<int>(std::round(fpsDisplay));
        std::string fpsText = "FPS: " + std::to_string(fpsInt);

        float marginX = 0.02f * w;
        float offsetY = compact ? 0.12f : 0.15f; // pull closer when small
        float y = h - offsetY * h;

        float fpsScale = textScale * 0.6f;   // 60% of title size
        if (fpsScale < 0.25f) fpsScale = 0.25f; // clamp so it doesn't vanish

        r.drawText(
            f2,
            fpsText,
            marginX,
            y,
            fpsScale,
            glm::vec3(0.2f, 0.8f, 1.0f) // bluish
        );
    }
}


void GameManager :: imGUIlessRender(Renderer& r, Input& input, GLFWwindow* window, int w, int h) {
    r.initFBO(w, h);
    r.bindFBO();
    r.resize(w, h);
    r.clear(0.06f, 0.06f, 0.07f);
    render(r, input, window, w, h);
    r.setTextProjection(w, h);

    r.unbindFBO();
    glViewport(0, 0, w, h);
    r.background_render(r.getFBOTexture(), "tex", 0);
}

bool GameManager::handleClick(float nx, float ny) {
    auto it = scenes.find(currentScene);
    if (it == scenes.end()) return false;
    const Scene& sc = it->second;

    for (const auto& cr : sc.clicks) {
        bool hit = (nx >= cr.x && nx <= cr.x + cr.w && ny >= cr.y && ny <= cr.y + cr.h);
        logger.log(std::string("hit-test ") + (cr.name.empty() ? "<noname>" : cr.name) +
            (hit ? " 1" : " 0"));

        if (!hit) continue;

        logger.log("Click hit: start=" + std::to_string(cr.start) +
            " quit=" + std::to_string(cr.quit) +
            " goto='" + cr.gotoScene + "'");

        // Quit first (works in Menu and Play)
        if (cr.quit) {
            logger.log("[Quit] click region flagged quit=true");
            if (quitCallback) quitCallback();
            else requestedQuit = true;
            return true;
        }

        // In Menu: only start:true does anything (exactly how 'start' works)
        if (app == AppState::Menu) {
            if (cr.start) {
                startGame();
                if (!cr.gotoScene.empty()) requestScene(cr.gotoScene);
                else {
                    auto it1 = keyMap.find(GLFW_KEY_1);
                    if (it1 != keyMap.end()) requestScene(it1->second);
                }
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
void GameManager::startGame() {
    app = AppState::Playing;
}

void GameManager::goToMenu() {
    app = AppState::Menu;
}
bool GameManager::popQuitRequested() {
    bool q = requestedQuit;
    requestedQuit = false;
    return q;
}

// Function used for the level editor to show current game objects
Scene* GameManager::getCurrentScene() {
    auto it = scenes.find(currentScene);
    if (it != scenes.end())
        return &it->second;
    return nullptr;
}

//function to get the unordered map for scenes
std::unordered_map<std::string, Scene>& GameManager::get_scenes_map() {
    return scenes;
}

void GameManager::update(float dt,Renderer& renderer,TextureStore& tex) {
    //update ECS animations
	const float fixedDt = 1.0f / 60.0f;
    animationSystem_.Update(fixedDt, ecs_, renderer, tex);
}

/**
 * @brief Pauses the game, changing the application state to Paused.
 * @param void
 * @returns void
 */
void GameManager::pauseGame() {
    if (app == AppState::Playing) {
        app = AppState::Paused;
    }
}

/**
 * @brief Resumes the game, changing the application state to Playing.
 * @param void
 * @returns void
 */
void GameManager::resumeGame() {
    if (app == AppState::Paused) {
        app = AppState::Playing;
    }
}

bool GameManager::reloadCurrentScene(TextureStore& tex) {
    if (currentScene.empty() || scenesFilePath.empty()) {
        logger.warning("[GameManager] Cannot reload: no current scene");
        return false;
    }

    std::string sceneToReload = currentScene;

    logger.log("[GameManager] Reloading scene: " + sceneToReload);

    // Read the entire scenes file again
    std::string jsonContent;
    if (!readFile(scenesFilePath.c_str(), jsonContent)) {
        logger.error("[GameManager] Failed to read scenes file for reload");
        return false;
    }

    rapidjson::Document d;
    d.Parse(jsonContent.c_str());
    if (d.HasParseError() || !d.IsObject()) {
        logger.error("[GameManager] Failed to parse scenes file for reload");
        return false;
    }

    // Find the current scene in the JSON
    if (!d.HasMember("scenes") || !d["scenes"].IsObject()) {
        logger.error("[GameManager] No scenes object in JSON");
        return false;
    }

    const auto& jsScenes = d["scenes"];
    if (!jsScenes.HasMember(sceneToReload.c_str())) {
        logger.error("[GameManager] Scene not found: " + sceneToReload);
        return false;
    }

    const auto& jsS = jsScenes[sceneToReload.c_str()];
    if (!jsS.IsObject()) {
        logger.error("[GameManager] Invalid scene object");
        return false;
    }

    // Load prefab defaults (same as in load() function)
    std::unordered_map<std::string, PrefabDef> prefabDefaults;
    // ... (copy the prefab loading code from load() function) ...

    // Clear and rebuild the scene
    Scene freshScene;

    // Background
    if (jsS.HasMember("background") && jsS["background"].IsString()) {
        std::string bg = toAssetsPath(jsS["background"].GetString());
        freshScene.background = tex.get(bg);
    }

    // Entities (same loading logic as in load() function)
    if (jsS.HasMember("entities") && jsS["entities"].IsArray()) {
        for (const auto& e : jsS["entities"].GetArray()) {
            if (!e.IsObject()) continue;

            PrefabDef def;
            DrawItem di{};

            // Copy the entity loading logic from load() function
            // (lines 237-292 in the current load() function)
            // ... entity loading code here ...

            freshScene.items.push_back(di);
        }
    }

    // Replace the current scene's items with the freshly loaded ones
    scenes[sceneToReload] = freshScene;

    logger.log("[GameManager] Scene reloaded successfully: " +
        std::to_string(freshScene.items.size()) + " items");

    return true;
}
/**
 * Current printing method for hamobjects components.
 *  it is incredibly lazy
 * \param id
 * \param overridesObj
 * \param alloc
 */
void GameManager::writeAllComponentsToJSON(EntityID id, rapidjson::Value& obj, rapidjson::Document::AllocatorType& alloc) {

    if (ecs_.getComponent<AudioData>(id))ecs_.getComponent<AudioData>(id)->ToJSON(obj, alloc);
    if (ecs_.getComponent<Transform>(id))ecs_.getComponent<Transform>(id)->ToJSON(obj, alloc);
    if (ecs_.getComponent<ImageRenderer>(id))ecs_.getComponent<ImageRenderer>(id)->ToJSON(obj, alloc);
    if (ecs_.getComponent<CircleRenderer>(id))ecs_.getComponent<CircleRenderer>(id)->ToJSON(obj, alloc);
    if (ecs_.getComponent<RectRenderer>(id))ecs_.getComponent<RectRenderer>(id)->ToJSON(obj, alloc);
    if (ecs_.getComponent<AnimalBehavior>(id))ecs_.getComponent<AnimalBehavior>(id)->ToJSON(obj, alloc);
    if (ecs_.getComponent<PhysicsComponent>(id))ecs_.getComponent<PhysicsComponent>(id)->ToJSON(obj, alloc);

    if (ecs_.getComponent<AIComponent>(id))ecs_.getComponent<AIComponent>(id)->ToJSON(obj, alloc);

    if (ecs_.getComponent<BoxCollider>(id))ecs_.getComponent<BoxCollider>(id)->ToJSON(obj, alloc);
}
