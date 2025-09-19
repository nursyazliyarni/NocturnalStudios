#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <cstdio>
#include <string>  // needed for std::string
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "../include/core/ImGuiManager.h"

Logger logger;

int main() {
    // Defaults
    int         cfgWidth = 1280;
    int         cfgHeight = 720;
    std::string cfgTitle = "NocturnalStudios";
    bool        cfgVsync = true;

    // Try load config.json
    if (FILE* fp = fopen("assets/config.json", "rb")) {
       // char buffer[65536];
        //rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
        std::vector<char> buf(1u << 16);                // 64 KB on heap
        rapidjson::FileReadStream is(fp, buf.data(), buf.size());
        rapidjson::Document doc;
        doc.ParseStream(is);
        fclose(fp);

        if (!doc.HasParseError() && doc.HasMember("window") && doc["window"].IsObject()) {
            const auto& w = doc["window"];
            if (w.HasMember("width") && w["width"].IsInt())     cfgWidth = w["width"].GetInt();
            if (w.HasMember("height") && w["height"].IsInt())    cfgHeight = w["height"].GetInt();
            if (w.HasMember("title") && w["title"].IsString())  cfgTitle = w["title"].GetString(); // copy = safe
            if (w.HasMember("vsync") && w["vsync"].IsBool())    cfgVsync = w["vsync"].GetBool();
        }
    }

    if (!glfwInit()) { std::fprintf(stderr, "GLFW init failed\n"); return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* win = glfwCreateWindow(cfgWidth, cfgHeight, cfgTitle.c_str(), nullptr, nullptr);
    if (!win) { std::fprintf(stderr, "Window creation failed\n"); glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(cfgVsync ? 1 : 0);   // set in JSON

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::fprintf(stderr, "GLEW init error\n"); return -1; }
    glGetError(); // clear harmless GL error from glewInit

	ImGuiManager imGuiManager(win);

    auto last = std::chrono::high_resolution_clock::now();
    int frames = 0; double acc = 0.0;

    while (!glfwWindowShouldClose(win)) {
        auto now = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration<double>(now - last).count();
        last = now; acc += dt; frames++;

        if (acc >= 1.0) {
            char title[256];
            std::snprintf(title, sizeof(title), "%s | FPS: %d", cfgTitle.c_str(), frames);
            glfwSetWindowTitle(win, title);
            frames = 0; acc = 0.0;
        }

        glClearColor(0.1f, 0.1f, 0.12f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

		imGuiManager.startFrame();

        ImGui::Begin("Hello");
        ImGui::Text("ImGui is working!");
        ImGui::End();

		// @Todo (axewhy): replace all the overlay with a single function call to ImGuiManager
		imGuiManager.showMousePosOverlayUI();
        imGuiManager.showPerformanceOverlayUI(dt);
		imGuiManager.showLogsOverlayUI();
		imGuiManager.showEntityListUI();
		imGuiManager.showTransformUI();

		imGuiManager.endFrame();


        glfwSwapBuffers(win);
        glfwPollEvents();
    }


    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}