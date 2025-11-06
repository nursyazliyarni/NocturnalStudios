/*!------------------------------------------------------------------------------------------------
    File: Renderer.cpp
    Project: NocturnalStudio

    This file contains ...

     Primary Author:
         Rishika Bimal Attodi
        -  Implemented sprite init/draw (excluding background), per-sprite rotation & scaling, 
        and front/behind passes.
     
    Secondary Author:
         Lau Xiang Yang
         - Implemented FBO

    Secondary Author
        Nur Syazliyarni 5%
        - Added the debugging lines for drawAllSprites



    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// OpenGL draw API (clear, drawSprite, debug lines)

#include "graphics/Renderer.h"
#include <GLFW/glfw3.h>   // brings gl.h on Windows
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "graphics/DebugDraw.h"
#include "resources/Font.h"

namespace {
    constexpr float TWO_PI = 2.0f * glm::pi<float>();

    // helper used only inside this file: filled circle via triangle fan
    inline void filledCircle(float cx, float cy, float r, int segments = 64) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= segments; ++i) {
            float a = (i * TWO_PI) / static_cast<float>(segments);
            glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
        }
        glEnd();
    }
}

Renderer::Renderer(const char* vertex_path, const char* fragment_path):render_shader(vertex_path,fragment_path) {
    logger.log("vertex shader path: " + std::string(vertex_path));
    logger.log("fragment shader path: " + std::string(fragment_path));

    float vertices[] = {
        -1.0f,  1.0f,            0.0f, 0.0f,
        -1.0f, -1.0f,            0.0f, 1.0f,
         1.0f,  1.0f,            1.0f, 0.0f,
         1.0f, -1.0f,            1.0f, 1.0f
    };

    glGenVertexArrays(1, &VAO_arr);
    glBindVertexArray(VAO_arr);

    glGenBuffers(1, &VBO_arr);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_arr);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void Renderer::background_render(GLuint texture_ID, const GLchar* uniform_VARNAME)
{
    render_shader.user_Shader_program();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glUniform1i(glGetUniformLocation(render_shader.get_programID(), uniform_VARNAME), 0);

    glBindVertexArray(VAO_arr);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Renderer::init2D(int w, int h) {
    resize(w, h);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::resize(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);   // top-left origin for fixed pipeline draws
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Renderer::clear(float r, float g, float b) {
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::tri(float x1, float y1, float x2, float y2, float x3, float y3) {
    glLoadIdentity();
    glBegin(GL_TRIANGLES);
    glColor3f(1.f, 0.f, 0.f); glVertex2f(x1, y1);
    glColor3f(0.f, 1.f, 0.f); glVertex2f(x2, y2);
    glColor3f(0.f, 0.f, 1.f); glVertex2f(x3, y3);
    glEnd();
}

void Renderer::rect(float x, float y, float w, float h) {
    glLoadIdentity();
    glBegin(GL_TRIANGLES);
    glColor3f(0.9f, 0.8f, 0.2f);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h);
    glVertex2f(x, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

void Renderer::line(float x1, float y1, float x2, float y2) {
    glLoadIdentity();
    glBegin(GL_LINES);
    glColor3f(1.f, 1.f, 1.f);
    glVertex2f(x1, y1); glVertex2f(x2, y2);
    glEnd();
}

void Renderer::rectOutline(float x, float y, float w, float h) {
    glLoadIdentity();
    glBegin(GL_LINE_LOOP);
    glColor3f(1.f, 1.f, 1.f);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void Renderer::circle(float cx, float cy, float r, int segments) {
    glLoadIdentity();
    glBegin(GL_LINE_LOOP);
    glColor3f(1.f, 1.f, 1.f);
    for (int i = 0; i < segments; ++i) {
        float a = (i * TWO_PI) / static_cast<float>(segments);
        glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
    }
    glEnd();
}

void Renderer::texturedTri(GLuint tex, float x, float y, float w, float h) {
    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.f, 0.f); glVertex2f(x, y);
    glTexCoord2f(1.f, 0.f); glVertex2f(x + w, y);
    glTexCoord2f(1.f, 1.f); glVertex2f(x + w, y + h);

    glTexCoord2f(0.f, 0.f); glVertex2f(x, y);
    glTexCoord2f(1.f, 1.f); glVertex2f(x + w, y + h);
    glTexCoord2f(0.f, 1.f); glVertex2f(x, y + h);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void Renderer::initFBO(int width, int height) {
    fboWidth = width;
    fboHeight = height;

    if (viewportFBO)     glDeleteFramebuffers(1, &viewportFBO);
    if (viewportTexture) glDeleteTextures(1, &viewportTexture);

    glGenFramebuffers(1, &viewportFBO);
    glGenTextures(1, &viewportTexture);

    glBindTexture(GL_TEXTURE_2D, viewportTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, viewportFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, viewportTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::bindFBO() const {
    glBindFramebuffer(GL_FRAMEBUFFER, viewportFBO);
    glViewport(0, 0, fboWidth, fboHeight);
}
void Renderer::unbindFBO() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint Renderer::getFBOTexture() const {
    return viewportTexture;
}

void Renderer::drawViewportShapes(int w, int h) {
    resize(w, h);
    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);

    float scale = 0.8f;
    float cx = w * 0.5f;
    float cy = h * 0.5f;

    tri(
        cx + (w * 0.50f - cx) * scale, cy + (h * 0.20f - cy) * scale,
        cx + (w * 0.20f - cx) * scale, cy + (h * 0.80f - cy) * scale,
        cx + (w * 0.80f - cx) * scale, cy + (h * 0.80f - cy) * scale
    );

    {
        float r = h * 0.10f;
        float margin = w * 0.05f;
        float cx_ = w - (r + margin);
        if (cx_ < r) cx_ = r;
        float cy_ = h * 0.20f;

        glColor3f(0.f, 0.5f, 1.f);
        filledCircle(cx_, cy_, r, 64);
    }

    {
        float pad = w * 0.01f;
        float bw = w * 0.35f;
        float bh = h * 0.5f;
        rectOutline(pad, pad, bw, bh);
    }
}

void Renderer::drawAllSprites(Texture& tex,
    const std::vector<int>& frontIdxs,
    bool behindPass,
    int viewportW, int viewportH)
{
    drawAllSprites(tex, frontIdxs, behindPass, /*scales*/ nullptr, /*rotations*/ nullptr, viewportW, viewportH);
}

void Renderer::initAllSprites() {
    sprite_vector.push_back({ 1, 0.01f, 0.10f, 0.15f, 0.25f, 0.0f });
    sprite_vector.push_back({ 2, 0.15f, 0.10f, 0.15f, 0.25f, 0.0f });
    sprite_vector.push_back({ 3, 0.40f, 0.35f, 0.25f, 0.30f, 0.0f });
    sprite_vector.push_back({ 4, 0.70f, 0.25f, 0.25f, 0.30f, 0.0f });
}

void Renderer::drawAllSprites(Texture& tex,
    const std::vector<int>& frontIdxs,
    bool behindPass,
    const float* /*scales*/,
    const float* /*rotations*/,
    int viewportW, int viewportH)
{
    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);

    for (int i = 0; i < (int)sprite_vector.size(); ++i) {
        bool isFront = std::find(frontIdxs.begin(), frontIdxs.end(), sprite_vector[i].idx) != frontIdxs.end();
        if (behindPass && isFront)   continue;
        if (!behindPass && !isFront) continue;

        float s = 1.0f;

        float x = sprite_vector[i].x * viewportW;
        float y = sprite_vector[i].y * viewportH;
        float w = sprite_vector[i].w * viewportW * s;
        float h = sprite_vector[i].h * viewportH * s;

        float cx = x + w * 0.5f;
        float cy = y + h * 0.5f;

        float cosA = std::cos(sprite_vector[i].angle);
        float sinA = std::sin(sprite_vector[i].angle);

        float x0, y0, x1, y1, x2, y2, x3, y3;

        float dx = x - cx;
        float dy = y - cy;
        x0 = cx + dx * cosA - dy * sinA;
        y0 = cy + dx * sinA + dy * cosA;

        dx = (x + w) - cx;
        dy = y - cy;
        x1 = cx + dx * cosA - dy * sinA;
        y1 = cy + dx * sinA + dy * cosA;

        dx = (x + w) - cx;
        dy = (y + h) - cy;
        x2 = cx + dx * cosA - dy * sinA;
        y2 = cy + dx * sinA + dy * cosA;

        dx = x - cx;
        dy = (y + h) - cy;
        x3 = cx + dx * cosA - dy * sinA;
        y3 = cy + dx * sinA + dy * cosA;

        GLuint t = tex.get_textureID((size_t)sprite_vector[i].idx);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, t);

        glBegin(GL_TRIANGLES);
        glTexCoord2f(0.f, 0.f); glVertex2f(x0, y0);
        glTexCoord2f(1.f, 0.f); glVertex2f(x1, y1);
        glTexCoord2f(1.f, 1.f); glVertex2f(x2, y2);
        glTexCoord2f(0.f, 0.f); glVertex2f(x0, y0);
        glTexCoord2f(1.f, 1.f); glVertex2f(x2, y2);
        glTexCoord2f(0.f, 1.f); glVertex2f(x3, y3);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

        if (Debug::Drawer::Instance().IsActive())
        {
            Debug::Drawer::Instance().AddLine(x0, y0, x1, y1, 0xFF00FFFF);
            Debug::Drawer::Instance().AddLine(x1, y1, x2, y2, 0xFF00FFFF);
            Debug::Drawer::Instance().AddLine(x2, y2, x3, y3, 0xFF00FFFF);
            Debug::Drawer::Instance().AddLine(x3, y3, x0, y0, 0xFF00FFFF);
        }

        Debug::Drawer::Instance().Render(Matrix<3>().Identity());
    }
}

std::vector<Renderer::sprite>& Renderer::get_sprite_vector() {
    return sprite_vector;
}

// ---- Text rendering controlled by Renderer ----
void Renderer::setTextShader(Shader& shader) {
    text_shader = &shader;
    text_shader->user_Shader_program();
    text_shader->SET_uniform_variable("text", 0); // sampler binding
    glUseProgram(0);
}

void Renderer::setTextProjection(int w, int h) {
    if (!text_shader) return;
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(w),
                                      0.0f, static_cast<float>(h));
    text_shader->user_Shader_program();
    glUniformMatrix4fv(
        glGetUniformLocation(text_shader->get_programID(), "projection"),
        1, GL_FALSE, &projection[0][0]
    );
    glUseProgram(0);
}

void Renderer::drawText(const Font& font, const std::string& text, float x, float y, float scale, const glm::vec3& color) {
    if (!text_shader) return;

    // text shader uses its own projection (bottom-left origin)
    text_shader->user_Shader_program();
    text_shader->SET_uniform_variable("textColor", color.x, color.y, color.z);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font.GetVAO());

    const auto& chars = font.GetCharacters();

    for (char c : text) {
        auto it = chars.find(c);
        if (it == chars.end()) continue;

        const Character& ch = it->second;

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        const float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, font.GetVBO());
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}


