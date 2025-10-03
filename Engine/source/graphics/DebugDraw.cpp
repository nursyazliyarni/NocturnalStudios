//#include "Precompiled.h"
#include "../include/graphics/DebugDraw.h"
#include "../include/graphics/Renderer.h"
#include <GL/glew.h>  

namespace Debug
{
    void Drawer::DebugDrawRectangle(float x, float y, float w, float h, unsigned int color)
    {
        float r = ((color >> 24) & 0xFF) / 255.0f;
        float g = ((color >> 16) & 0xFF) / 255.0f;
        float b = ((color >> 8) & 0xFF) / 255.0f;
        float a = (color & 0xFF) / 255.0f;

        glColor4f(r, g, b, a);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);     // bottom-left
        glVertex2f(x + w, y);     // bottom-right
        glVertex2f(x + w, y + h);   // top-right
        glVertex2f(x, y + h);   // top-left
        glEnd();
    }

    void Drawer::DebugDrawCircle(float cx, float cy, float r, unsigned int color, int segments)
    {
        float R = ((color >> 24) & 0xFF) / 255.0f;
        float G = ((color >> 16) & 0xFF) / 255.0f;
        float B = ((color >> 8) & 0xFF) / 255.0f;
        float A = (color & 0xFF) / 255.0f;

        glColor4f(R, G, B, A);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; i++) {
            float theta = (2.0f * 3.14159f * i) / segments;
            glVertex2f(cx + r * cosf(theta), cy + r * sinf(theta));
        }
        glEnd();
    }
}
