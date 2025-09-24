// OpenGL draw API (clear, drawSprite, debug lines)

#include "graphics/Renderer.h"
#include <GLFW/glfw3.h>   // brings gl.h on Windows
#include <cmath>
#include <glm/gtc/constants.hpp>

namespace {
    constexpr float TWO_PI = 2.0f * glm::pi<float>();
}


void Renderer::init2D(int w, int h) {
    resize(w, h);                 // set pixel-space projection

    // sensible defaults for 2D
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure we’re not in wireframe
}

void Renderer::resize(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);   // left, right, bottom, top, near, far
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
    glBegin(GL_TRIANGLES); // two triangles make a quad
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

