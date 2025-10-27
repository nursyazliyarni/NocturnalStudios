#include "core/GameStateManager.h"
#include "stb_image.h"
#include <rapidjson/document.h>
#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>

// ---------- TextureStore ----------
static GLuint loadGLTexture(const std::string& path) {
    int w,h,n;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 0);
    if (!data) return 0;

    GLenum fmt = (n==4)?GL_RGBA: (n==3)?GL_RGB: (n==1)?GL_RED: GL_RG;

    GLuint id=0; glGenTextures(1,&id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexImage2D(GL_TEXTURE_2D,0,fmt,w,h,0,fmt,GL_UNSIGNED_BYTE,data);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D,0);
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

// ---------- tiny JSON helpers ----------
static bool readFile(const char* p, std::string& out) {
    std::ifstream f(p); if (!f) return false;
    std::ostringstream ss; ss << f.rdbuf(); out = ss.str(); return true;
}



static std::string toAssetsPath(const std::string& p) {
    if (p.rfind("assets/", 0) == 0) return p;
    return "assets/" + p;  // scenes use "textures/.." to  make "assets/textures/.."
}
struct PrefabDef { float w = 0.2f, h = 0.2f, angle = 0.0f; };


// ---------- GameStateManager ----------
bool GameStateManager::load(const char* prefabsPath, const char* scenesPath, TextureStore& tex) {
    //// we only need scenes.json to render; prefabs.json is used by your ECS/Factory.
    //std::string s; if (!readFile(scenesPath, s)) return false;
    //rapidjson::Document d; d.Parse(s.c_str()); if (d.HasParseError()) return false;

    //// states
    //currentScene = d["states"]["initial"].GetString();
    //const auto& km = d["states"]["keymap"];
    //auto mapOne = [&](const char* k, int glfwKey) {
    //    if (km.HasMember(k)) keyMap[glfwKey] = km[k].GetString();
    //};
    //mapOne("1", GLFW_KEY_1); mapOne("2", GLFW_KEY_2);
    //mapOne("3", GLFW_KEY_3); mapOne("4", GLFW_KEY_4);
    //if (km.HasMember("ESC")) backKey = km["ESC"].GetString();

    //// scenes
    //const auto& jsScenes = d["scenes"];
    //for (auto it = jsScenes.MemberBegin(); it != jsScenes.MemberEnd(); ++it) {
    //    const char* name = it->name.GetString();
    //    const auto& jsS = it->value;

    //    Scene sc;
    //    sc.background = tex.get(jsS["background"].GetString());

    //    const auto& ents = jsS["entities"];
    //    for (auto& e : ents.GetArray()) {
    //        const auto& ov = e["overrides"];
    //        DrawItem di;
    //        di.tex   = tex.get(ov["sprite_path"].GetString());
    //        di.x     = ov["x"].GetFloat();
    //        di.y     = ov["y"].GetFloat();
    //        di.w     = ov["w"].GetFloat();
    //        di.h     = ov["h"].GetFloat();
    //        di.angle = ov.HasMember("angle") ? ov["angle"].GetFloat() : 0.0f;
    //        sc.items.push_back(di);

    //        // --- clickable buttons (optional per-entity) ---
    //        ClickRegion cr;
    //        cr.x = di.x; cr.y = di.y; cr.w = di.w; cr.h = di.h;
    //        cr.quit = (e.HasMember("quit") && e["quit"].IsBool() && e["quit"].GetBool());
    //        if (e.HasMember("goto") && e["goto"].IsString())
    //            cr.gotoScene = e["goto"].GetString();
    //        if (cr.quit || !cr.gotoScene.empty())
    //            sc.clicks.push_back(std::move(cr));

    //    }
    //    scenes[name] = std::move(sc);
    //}
    //return true;

        // ---- 1) read prefab defaults (w,h,angle) ----
    std::unordered_map<std::string, PrefabDef> prefabDefaults;
    {
        std::string s;
        if (readFile(prefabsPath, s)) {
            rapidjson::Document d; d.Parse(s.c_str());
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
    rapidjson::Document d; d.Parse(s.c_str());
    if (d.HasParseError() || !d.IsObject()) return false;

    // states
    if (d.HasMember("states") && d["states"].IsObject()) {
        const auto& st = d["states"];
        if (st.HasMember("initial") && st["initial"].IsString())
            currentScene = st["initial"].GetString();

        if (st.HasMember("keymap") && st["keymap"].IsObject()) {
            const auto& km = st["keymap"];
            auto mapKey = [&](const char* keyStr, int glfwKey) {
                if (km.HasMember(keyStr) && km[keyStr].IsString())
                    keyMap[glfwKey] = km[keyStr].GetString();
                };
            mapKey("1", GLFW_KEY_1);
            mapKey("2", GLFW_KEY_2);
            mapKey("3", GLFW_KEY_3);
            mapKey("4", GLFW_KEY_4);
            if (km.HasMember("ESC") && km["ESC"].IsString())
                backKey = km["ESC"].GetString();
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
                if (e.HasMember("prefab") && e["prefab"].IsString()) {
                    auto itp = prefabDefaults.find(e["prefab"].GetString());
                    if (itp != prefabDefaults.end()) def = itp->second;
                }

                DrawItem di{};
                di.w = def.w; di.h = def.h; di.angle = def.angle;

                if (e.HasMember("overrides") && e["overrides"].IsObject()) {
                    const auto& ov = e["overrides"];
                    if (ov.HasMember("x") && ov["x"].IsNumber()) di.x = ov["x"].GetFloat();
                    if (ov.HasMember("y") && ov["y"].IsNumber()) di.y = ov["y"].GetFloat();
                    if (ov.HasMember("w") && ov["w"].IsNumber()) di.w = ov["w"].GetFloat();
                    if (ov.HasMember("h") && ov["h"].IsNumber()) di.h = ov["h"].GetFloat();
                    if (ov.HasMember("angle") && ov["angle"].IsNumber()) di.angle = ov["angle"].GetFloat();
                    if (ov.HasMember("sprite_path") && ov["sprite_path"].IsString()) {
                        std::string sp = toAssetsPath(ov["sprite_path"].GetString());
                        di.tex = tex.get(sp);
                    }
                }

                sc.items.push_back(di);

                // clickable?
                ClickRegion cr;
                cr.x = di.x; cr.y = di.y; cr.w = di.w; cr.h = di.h;
                if (e.HasMember("goto") && e["goto"].IsString()) cr.gotoScene = e["goto"].GetString();
                if (e.HasMember("quit") && e["quit"].IsBool())   cr.quit = e["quit"].GetBool();
                if (cr.quit || !cr.gotoScene.empty()) sc.clicks.push_back(cr);
            }
        }

        scenes[sceneName] = std::move(sc);
    }

    if (currentScene.empty() && !scenes.empty())
        currentScene = scenes.begin()->first;

    return true;

}

void GameStateManager::requestScene(const std::string& name) {
    if (scenes.count(name)) currentScene = name;
}

void GameStateManager::handleKeyPress(int key) {
    auto it = keyMap.find(key);
    if (it != keyMap.end()) requestScene(it->second);
    if (key == GLFW_KEY_ESCAPE) requestScene(backKey);
}

void GameStateManager::render(Renderer& r, int w, int h) const {
    //auto it = scenes.find(currentScene);
    //if (it == scenes.end()) return;
    //const Scene& sc = it->second;

    //// background (fullscreen)
    //r.background_render(sc.background, "img"); // your shader uniform name

    //// entities
    //for (const auto& di : sc.items) {
    //    float px = di.x * w, py = di.y * h, pw = di.w * w, ph = di.h * h;
    //    // if you want rotation, adapt your rotated-quad from Renderer::drawAllSprites
    //    r.texturedTri(di.tex, px, py, pw, ph);
    //}
    auto it = scenes.find(currentScene);
    if (it == scenes.end()) return;
    const Scene& sc = it->second;

    // background (fullscreen) — NOTE uniform must be "bg_texture"
    r.background_render(sc.background, "bg_texture");

    // entities (normalized ? pixels)
    for (const auto& di : sc.items) {
        float px = di.x * w, py = di.y * h, pw = di.w * w, ph = di.h * h;
        r.texturedTri(di.tex, px, py, pw, ph);
    }

}



bool GameStateManager::handleClick(float nx, float ny) {
    auto it = scenes.find(currentScene);
    if (it == scenes.end()) return false;
    const Scene& sc = it->second;


    for (const auto& cr : sc.clicks) {
        if (nx >= cr.x && nx <= cr.x + cr.w &&
            ny >= cr.y && ny <= cr.y + cr.h) {
            if (cr.quit) {
                requestedQuit = true;
            }

            else if (!cr.gotoScene.empty()) {
                requestScene(cr.gotoScene);
            }
            return true;
        }
    }
    return false;
}

bool GameStateManager::popQuitRequested() {
    bool q = requestedQuit;
    requestedQuit = false;
    return q;
}
