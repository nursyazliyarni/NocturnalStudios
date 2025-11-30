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
#include <rapidjson/istreamwrapper.h>
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


namespace {
    //build collider AABB in screen-pixel space from DrawItem
    AABB BuildColliderAABB(const DrawItem& di, int screenW, int screenH)
    {
        if (!di.hasCollider) {
            //zero-size box (won't be hit)
            return AABB(Vector2D(0.f, 0.f), Vector2D(0.f, 0.f));
        }

        Transform* t = ECS::getECS().getComponent<Transform>(di.entityID);
        if (!t)return {};

        //center of collider in normalized coords
        float cxNorm = t->position.x() + di.colOffsetX;
        float cyNorm = t->position.y() + di.colOffsetY;

        //size in pixels
        float wPix = di.colW * screenW;
        float hPix = di.colH * screenH;

        //convert to pixel center
        float cxPix = cxNorm * screenW;
        float cyPix = cyNorm * screenH;
        Vector2D pos(cxPix - wPix * 0.5f, cyPix - hPix * 0.5f);
        Vector2D size(wPix, hPix);
        return AABB(pos, size);
    }
}

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
bool GameManager::checkForSceneChange() {
    if (scene_changed_since_click)
    {
        scene_changed_since_click = false;
        return true;
    }
    else return false;
}

bool GameManager::saveAll(const char* saveJsonPath, TextureStore& tex) {
    return true;
}

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
                rapidjson::Value overrides(rapidjson::kArrayType);
                if (ecs_.findEntityByName(i.name)!=INVALID_ENTITY_ID&&!ecs_.getEntityComponents(ecs_.findEntityByName(i.name)).empty())
                {
                    logger.log("Overrides for object: " + i.name+" found in ECS entity "+ std::to_string(ecs_.findEntityByName(i.name)));
                    writeAllComponentsToJSONArray(ecs_.findEntityByName(i.name), overrides, alloc);
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



bool GameManager::loadAll(PrefabManager& prefabs, const char* scenesPath, TextureStore& tex) {

    app = AppState::Menu;   // lock number keys until Play is pressed
    requestedQuit = false;  // (optional) clear quit flag at load

    // Store paths for later reload
    scenesFilePath = scenesPath;

    //read file
    std::ifstream f(scenesPath);
    if (!f) return false;

    rapidjson::IStreamWrapper isw(f);

    rapidjson::Document d;
    d.ParseStream(isw);

    if (d.HasParseError() || !d.IsObject()) return false;
    logger.log("file parsed");
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
    if (!d.HasMember("scenes") || !d["scenes"].IsArray()) return false;

    unsigned int unnamd_scenes{},unnamed_entities{};
    for (rapidjson::Value& scn:d["scenes"].GetArray())
    {
        logger.log("reading scene");
        std::string scn_name{};
        if (scn.HasMember("name") && scn["name"].IsString())scn_name = scn["name"].GetString();
        else {
            scn_name = "Scene_" + std::to_string(unnamd_scenes);
            unnamd_scenes++;
        }

        Scene new_scene;
        if (scn.HasMember("background") && scn["background"].IsString())
        {
            std::string path = scn["background"].GetString();
            if (path.rfind("assets/", 0) != 0) path = "assets/" + path;
            new_scene.background  =tex.get(path);
            logger.log("bg: " + std::to_string(new_scene.background));
        }

        
        if (scn.HasMember("entities")&&scn["entities"].IsArray())
        {
            logger.log("adding entity");
            for (rapidjson::Value& ent : scn["entities"].GetArray())
            {
                EntityID new_entity{ INVALID_ENTITY_ID };
                std::string ent_name{};

                if (ent.HasMember("name") && ent["name"].IsString()) {
                    //check if already have an entity named that lmao
                    std::string base = ent["name"].GetString();
                    std::string check_name = base;
                    unsigned int suffix = 0;
                    while (ecs_.findEntityByName(check_name) != INVALID_ENTITY_ID) {
                        check_name = base + "_" + std::to_string(suffix++);
                    }
                    ent_name = check_name;
                }
                else {
                    //same for unnamed sntity
                    std::string base = "Entity_" + std::to_string(unnamed_entities);
                    std::string check_name = base;
                    unsigned int suffix = 0;
                    while (ecs_.findEntityByName(check_name) != INVALID_ENTITY_ID) {
                        check_name = base + "_" + std::to_string(suffix++);
                    }
                    // ensure the global counter advances to avoid reusing base next time
                    unnamed_entities++;
                    ent_name = check_name;
                }
                //name resolved yay
                logger.log("entity name: " +ent_name);

                if (ent.HasMember("prefab")&&ent["prefab"].IsString())
                {
                    logger.log("Creating entity from prefab");
                    new_entity = prefabs.CreateEntityFromPrefab(ent["prefab"].GetString(), ent_name);
                    logger.log(ent_name+" created");
                    //process overrides (normal components)
                    //uses FromJson
                    if (ent.HasMember("overrides") && ent["overrides"].IsArray())
                    {
                        logger.log("adding components");
                        for (rapidjson::Value& ovr : ent["overrides"].GetArray()) {
                            processComponentsFromJSON(new_entity, ovr,true);
                        }
                    }

                    
                }
                else //entity not based on prefabs
                {
                    new_entity = ecs_.createEntity(ent_name);

                    if (ent.HasMember("components") && ent["components"].IsArray())
                    {
                        logger.log("adding components");
                        for (rapidjson::Value& cmp : ent["components"].GetArray()) {
                            processComponentsFromJSON(new_entity, cmp);
                        }
                    }
                }

                if (new_entity == INVALID_ENTITY_ID) continue;
                //components done
                
                DrawItem di{};
                updateDrawItem(di, new_entity);

                if (ent.HasMember("prefab") && ent["prefab"].IsString())di.item_type = ent["prefab"].GetString();

                //other overrides
                if (ent.HasMember("is_draggable") && ent["is_draggable"].IsBool())di.is_draggable = ent["is_draggable"].GetBool();
                if (ent.HasMember("is_dragging") && ent["is_dragging"].IsBool())di.is_dragging = ent["is_dragging"].GetBool();

                new_scene.items.push_back(di);
                logger.log("Entity " + di.name + " with ID " + std::to_string(di.entityID) + " added to "+scn_name);

                //handle click regions now
                ClickRegion cr;
                Transform *t = ecs_.getComponent<Transform>(new_entity);
                ImageRenderer *ir= ecs_.getComponent<ImageRenderer>(new_entity); //will change to a new component called clickable
                if (!t||!ir) continue;

                cr.x = t->position.x(); cr.y = t->position.y(); cr.w = ir->width; cr.h = ir->height;
                cr.name = di.name;
                if (ent.HasMember("goto") && ent["goto"].IsString()) cr.gotoScene = ent["goto"].GetString();
                if (ent.HasMember("quit") && ent["quit"].IsBool())   cr.quit = ent["quit"].GetBool();
                if (ent.HasMember("start") && ent["start"].IsBool())  cr.start = ent["start"].GetBool();

                // store the region if it does ANY action
                if (cr.quit || cr.start || !cr.gotoScene.empty())
                    logger.log("cr: " + std::to_string(cr.x)+" " + std::to_string(cr.y) + " " + std::to_string(cr.w) + " " + std::to_string(cr.h) + " " +cr.name);
                    new_scene.clicks.push_back(cr);
            }
        }
        logger.log("scene added");
        scenes.emplace(scn_name, new_scene);
    }

    initialScenes_ = scenes;


    //temp: add a flamingo entity with sprite animation to ECS
    {
        //EntityID flamingo = ecs_.createEntity();

        //ecs_.addComponent<Transform>(flamingo, Transform{});
        //ecs_.addComponent<ImageRenderer>(flamingo, ImageRenderer{});
        //ecs_.addComponent<SpriteAnimationComponent>(flamingo, SpriteAnimationComponent{});

        //auto* t = ecs_.getComponent<Transform>(flamingo);
        //auto* img = ecs_.getComponent<ImageRenderer>(flamingo);
        //auto* anim = ecs_.getComponent<SpriteAnimationComponent>(flamingo);

        //t->position = { 0.1f, 0.7f };
        //t->scale = { 0.25f, 0.25f };

        //img->isVisible = true;
        //img->textureID = tex.get("assets/textures/flamingo_spritesheet.png");

        ////Define an animation sequence
        //SpriteAnimationComponent::Animation idle;
        //idle.frames = 5;
        //idle.frameTime = 0.12f;
        //idle.loop = true;
        //idle.spritePath = "assets/textures/flamingo_spritesheet.png";
        //idle.sheetColumns = 5;
        //idle.sheetRows = 1;

        //anim->animations["idle"] = idle;
        //anim->currentAnim = "idle";
        //anim->playing = true;
        //anim->currentFrame = 0;
        //anim->sheetColumns = 5;
        //anim->sheetRows = 1;
    }


    if (currentScene.empty() && !scenes.empty()) currentScene = scenes.begin()->first;
    return true;
}

void GameManager::requestScene(const std::string& name) {
    if (!scenes.count(name)) return;
    currentScene = name;
    scene_changed_since_click = true;
    if (onSceneChanged) onSceneChanged(currentScene);
}





void GameManager::resetAllScenesToInitial()
{
    // If we have a snapshot, restore it
    if (!initialScenes_.empty())
    {
        scenes = initialScenes_;
    }
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
    if (it == scenes.end()) { logger.log("scene not found");  return; }
    const Scene& sc = it->second;

    // background (fullscreen) NOTE uniform must be "bg_texture"
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

    //render items in scene, BASED ON di
    // entities (normalized ? pixels)
   // entities (normalized → pixels) — draw JSON **only** if there is NO ECS entity by the same name
    for (DrawItem di : sc.items) {
        if (di.entityID != INVALID_ENTITY_ID)
        {
            //logger.log("drawing " + di.name+" "+std::to_string(ecs_.findEntityByName(di.name)));
            Transform* t = ecs_.getComponent<Transform>(ecs_.findEntityByName(di.name));
            ImageRenderer* img = ecs_.getComponent<ImageRenderer>(ecs_.findEntityByName(di.name));

            if (!t || !img)continue;



            if (!img->isVisible || img->textureID <= 0)continue;

            if (auto* anim = ecs_.getComponent<SpriteAnimationComponent>(di.entityID)) {
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

            if (di.hasCollider && (di.colW <= 0.0f || di.colH <= 0.0f)) {
                di.colOffsetX = 0.0f;
                di.colOffsetY = 0.0f;
                di.colW = img->width;   //same size as sprite
                di.colH = img->height;
            }
            float px = t->position.x() * w, py = t->position.y() * h, pw = img->width * w, ph = img->height * h;
            if (di.name == "flamingo" || di.name == "seal" || di.name == "owl" || di.name == "horse") {
                AnimalBehavior* ab = ecs_.getComponent<AnimalBehavior>(di.entityID);
                //logger.log(di.name + " has animalbehavior: " + std::to_string(ab != nullptr));
                if (di.still_alive && ab) {
                    //logger.log(di.name + " has animalbehavior: " + std::to_string(ab != nullptr));
                    r.rect(w / 2, h / 2 - 200, ab->health_bar * 4, 25, 1, 0, 0);
                    r.rect(w / 2, h / 2 - 150, ab->hunger_bar * 4, 25, 0, 1, 0);
                    r.rect(w / 2, h / 2 - 100, ab->sanity_bar * 4, 25, 0, 0, 1);

                    Cam2D_Apply(currentScene, px, py, pw, ph, w, h);
                    r.texturedTri(img->textureID, px, py, pw, ph);
                }

            }
            else {
                Cam2D_Apply(currentScene, px, py, pw, ph, w, h);
                r.texturedTri(di.tex, px, py, pw, ph);

                // --- Collider outline (debug) ---
                if (/*Debug::Drawer::Instance().IsActive() &&*/ di.hasCollider)
                {
                    //collider center in NORMALIZED coords (same as di.x/di.y)
                    float cxNorm = t->position.x() + di.colOffsetX;
                    float cyNorm = t->position.y() + di.colOffsetY;

                    //collider size in NORMALIZED space
                    float cwNorm = di.colW;
                    float chNorm = di.colH;

                    //convert to pixels
                    float cw = cwNorm * w;
                    float ch = chNorm * h;
                    float cx = cxNorm * w;
                    float cy = cyNorm * h;

                    //apply same camera transform as sprites
                    Cam2D_Apply(currentScene, cx, cy, cw, ch, w, h);

                    float left = cx;
                    float bottom = cy;

                    //draw a green outline rectangle
                    glPushAttrib(GL_ALL_ATTRIB_BITS);
                    glUseProgram(0);
                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_DEPTH_TEST);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                    glColor3f(0.0f, 1.0f, 0.0f);  // green
                    glBegin(GL_LINE_LOOP);
                    glVertex2f(left, bottom);
                    glVertex2f(left + cw, bottom);
                    glVertex2f(left + cw, bottom + ch);
                    glVertex2f(left, bottom + ch);
                    glEnd();

                    glPopAttrib();
                }
                // --- end collider outline ---

                if (di.is_dragging && !duplicate_item) {
                    duplicate_item = std::make_unique<DrawItem>(di);
                }

                if (duplicate_item) {
                    Transform* dt = ecs_.getComponent<Transform>(duplicate_item->entityID);
                    ImageRenderer* dimg = ecs_.getComponent<ImageRenderer>(duplicate_item->entityID);
                    r.texturedTri(duplicate_item->tex, dt->position.x() * w, dt->position.y() * h, dimg->width * w, dimg->height * h);
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
            }
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
        static int    frameCount = 0;
        static double fpsDisplay = 0.0;

        // avoid division by zero
        if (dt <= 0.0) dt = 1e-6;

        // 🔹 IF your dt is in MILLISECONDS, convert to seconds here:
        double dtSeconds = dt;
        // if you know dt is in ms, UNCOMMENT this:
        // dtSeconds = dt * 0.001;   // ms → s

        accumTime += dtSeconds;
        ++frameCount;

        // Update FPS about once per second
        if (accumTime >= 1.0) {
            fpsDisplay = static_cast<double>(frameCount) / accumTime; // avg FPS
            accumTime = 0.0;
            frameCount = 0;
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

    if (m_isGameOver) {
        // draw a dark quad over the screen
        r.rect(0, 0, (float)w, (float)h, 0, 0, 0.0f); // or semi-transparent if your rect supports it

        // big “GAME OVER” in the middle
        if (s_fontTitle) {
            const Font& f = resourceManager.getFont(*s_fontTitle);
            r.drawText(f, "GAME OVER", w * 0.35f, h * 0.6f, 1.5f,
                glm::vec3(1.0f, 0.2f, 0.2f));
        }

        // you can also show “Press R to Restart” etc, handled with Input, not ImGui
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
    Scene& sc = it->second;

    // --- Sync click regions from DrawItem colliders ---
    for (auto& cr : sc.clicks)
    {
        if (cr.name.empty())
            continue;

        //find DrawItem whose name matches the click region
        auto diIt = std::find_if(
            sc.items.begin(), sc.items.end(),
            [&](const DrawItem& d) { return d.name == cr.name; });

        if (diIt == sc.items.end())
            continue;

        const DrawItem& di = *diIt;
        if (!di.hasCollider)
            continue;

        Transform* t = ecs_.getComponent<Transform>(di.entityID);
        ImageRenderer* ir = ecs_.getComponent<ImageRenderer>(di.entityID);
        if (!t||!ir)return false;

        //collider in normalised space
        float wNorm = (di.colW > 0.0f ? di.colW : ir->width);
        float hNorm = (di.colH > 0.0f ? di.colH : ir->height);

        float cxNorm = t->position.x() + di.colOffsetX;
        float cyNorm = t->position.y() + di.colOffsetY;

        float left = cxNorm;
        float bottom = cyNorm;

        //write back into ClickRegion so hit-test uses the collider
        cr.x = left;
        cr.y = bottom;
        cr.w = wNorm;
        cr.h = hNorm;
    }
	//--- end sync click regions ---

    logger.log("clicked at x: " + std::to_string(nx) + ", y: " + std::to_string(ny));

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

                // reset all scenes to the initial JSON values
                resetAllScenesToInitial();

                // enter Play mode
                startGame();

                if (!cr.gotoScene.empty()) {
                    requestScene(cr.gotoScene);
                }
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
//void GameManager::startGame() {
//    // enter play state
//    app = AppState::Playing;
//
//    // every time a new game starts, reset ALL animals in ALL scenes
//    for (auto& [name, scene] : scenes) {
//        for (auto& item : scene.items) {
//            if (item.item_type == "animal") {
//                // reset bars
//                item.health_bar        = 100.0f;
//                item.hunger_bar        = 100.0f;
//                item.sanity_bar        = 100.0f;
//
//                // reset timers
//                item.acc_timer         = 0.0f;
//                item.staring_countdown = 10.0f;
//
//                // reset status / drag flags
//                item.still_alive       = true;
//                item.is_dragging       = false;
//            }
//        }
//    }
//}

void GameManager::startGame() {
    app = AppState::Playing;
    m_isGameOver = false;
}
 


void GameManager::goToMenu() {
    app = AppState::Menu;
}
bool GameManager::popQuitRequested() {
    bool q = requestedQuit;
    requestedQuit = false;
    return q;
}

void GameManager::triggerGameOver() {
    logger.log("=== GAME OVER TRIGGERED ===");
    m_isGameOver = true;
    
    // Go to MainMenu (or GameOver scene if you have one)
    if (scenes.count("GameOver") > 0) {
        requestScene("GameOver");
    } else {
        requestScene("MainMenu");
    }
    app = AppState::Menu;
}

/*
void GameManager::triggerGameOver() {
    logger.log("=== GAME OVER TRIGGERED ===");

    // Option 1: Go to a GameOver scene (if you have one)
    if (scenes.count("GameOver") > 0) {
        requestScene("GameOver");
        app = AppState::Menu;  // Set to menu state
    }
    // Option 2: Go back to main menu
    else {
        requestScene("MainMenu");
        app = AppState::Menu;
    }
}
*/

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



bool GameManager::populateScene(const std::string& sceneName, PrefabManager& pm) {
    // check first if the scene has been populated
    auto it = scenes.find(sceneName);
    if (it == scenes.end())return false;

    if (!it->second.loaded)
    {
        //load it!
    }return true;
}

bool GameManager::reloadAllScene(TextureStore& tex) {
    if (scenes.empty() || scenesFilePath.empty()) {
        logger.warning("[GameManager] Cannot reload: no current scenes");
        return false;
    }

    std::string sceneToReload = currentScene;

    logger.log("[GameManager] Reloading scene: " + sceneToReload);
    for (const auto& pair : scenes) {
        const std::string& name = pair.first;  // the key
        const Scene& scene = pair.second;      // the value

        for (DrawItem di : pair.second.items)
        {
            ImageRenderer* img = ecs_.getComponent<ImageRenderer>(di.entityID);
            if (!img)continue;
            if (img->textureID <= 0 && !img->image_path.empty())
            {
                std::string path = img->image_path;
                if (path.rfind("assets/", 0) != 0) path = "assets/" + path;
                logger.log("Texture looking for " + path);
                img->textureID = tex.get(path);
                logger.log("Texture for " + di.name + " refreshed to " + std::to_string(img->textureID));
            }
        }
    }
    

    return true;
}
/**
 * Current printing method for objects components.
 *  it is incredibly lazy
 * \param id
 * \param overridesObj
 * \param alloc
 */
void GameManager::writeAllComponentsToJSONArray(EntityID id, rapidjson::Value& obj, rapidjson::Document::AllocatorType& alloc) {

    if (ecs_.getComponent<AudioData>(id)) { 
        rapidjson::Value entObj(rapidjson::kObjectType);
        ecs_.getComponent<AudioData>(id)->ToJSON(entObj, alloc); 
        obj.PushBack(entObj, alloc);
    }

    if (ecs_.getComponent<Transform>(id)) { 
        rapidjson::Value entObj(rapidjson::kObjectType);
        ecs_.getComponent<Transform>(id)->ToJSON(entObj, alloc);
        obj.PushBack(entObj, alloc);
    }

    if (ecs_.getComponent<ImageRenderer>(id)) { 
        rapidjson::Value entObj(rapidjson::kObjectType);
        ecs_.getComponent<ImageRenderer>(id)->ToJSON(entObj, alloc);
        obj.PushBack(entObj, alloc);
    }

    if (ecs_.getComponent<CircleRenderer>(id)){
        rapidjson::Value entObj(rapidjson::kObjectType);
        ecs_.getComponent<CircleRenderer>(id)->ToJSON(entObj, alloc);
        obj.PushBack(entObj, alloc);
    }

    if (ecs_.getComponent<RectRenderer>(id)) { 
        rapidjson::Value entObj(rapidjson::kObjectType);
        ecs_.getComponent<RectRenderer>(id)->ToJSON(entObj, alloc); 
        obj.PushBack(entObj, alloc);
    }

    if (ecs_.getComponent<AnimalBehavior>(id)) { 
        rapidjson::Value entObj(rapidjson::kObjectType);
        ecs_.getComponent<AnimalBehavior>(id)->ToJSON(entObj, alloc); 
        obj.PushBack(entObj, alloc);
    }

    if (ecs_.getComponent<PhysicsComponent>(id)) { 
        rapidjson::Value entObj(rapidjson::kObjectType);
        ecs_.getComponent<PhysicsComponent>(id)->ToJSON(entObj, alloc);
        obj.PushBack(entObj, alloc);
    }

    if (ecs_.getComponent<AIComponent>(id)) {
        rapidjson::Value entObj(rapidjson::kObjectType);
        ecs_.getComponent<AIComponent>(id)->ToJSON(entObj, alloc);
        obj.PushBack(entObj, alloc);
    }

    if (ecs_.getComponent<BoxCollider>(id)) { 
        rapidjson::Value entObj(rapidjson::kObjectType);
        ecs_.getComponent<BoxCollider>(id)->ToJSON(entObj, alloc); 
        obj.PushBack(entObj, alloc);
    }
}

void GameManager::processComponentsFromJSON(EntityID id, rapidjson::Value& obj,bool isOverride){
    if (!obj.HasMember("type") || !obj["type"].IsString())return;
    
    if (isOverride)
    {
    
        ComponentFactory::ParseFromName(id, obj["type"].GetString(), obj);
    }
    else {
        ComponentFactory::CreateFromName(id, obj["type"].GetString(), obj);
    }
    

}
void GameManager::updateDrawItem(DrawItem& d, EntityID e) {
    d.name = ecs_.getEntityName(e);

    if (ecs_.getComponent<ImageRenderer>(e))d.tex = ecs_.getComponent<ImageRenderer>(e)->textureID;

    d.entityID = e;
}
