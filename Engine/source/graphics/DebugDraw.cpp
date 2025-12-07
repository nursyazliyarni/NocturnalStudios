/*!------------------------------------------------------------------------------------------------
    File: DebugDraw.cpp
    Project: NocturnalStudio

    This file contains the definitions for drawing debug rectangles, lines and circles. It
    has the rendering functionality for only debugging mode

    Primary Author:
         Nur Syazliyarni 100%
        - Coded the debug rects, lines and circles

    Secondary Author:
         Name (%)
        -Brief

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

//#include "Precompiled.h"
#include "../include/graphics/DebugDraw.h"
#include "../include/graphics/Renderer.h"
#include <GL/glew.h>  

namespace Debug
{
    Drawer& Drawer::Instance()
    {
        static Drawer instance; // create one global Drawer object
        return instance;
    }

    void Drawer::AddRect(float x, float y, float w, float h, unsigned int color)
    {
        rects.push_back({ {x, y}, w, h, color }); // store a new rectangle to be drawn later
    }

    void Drawer::AddCircle(float cx, float cy, float r, unsigned int color, int segments)
    {
        circles.push_back({ {cx, cy}, r, color, segments }); // store a new circle
    }

    void Drawer::AddLine(float x1, float y1, float x2, float y2, unsigned int color)
    {
        lines.push_back({ {x1, y1}, {x2, y2}, color }); // store a line segment
    }

    /*
    void Drawer::AddSprite(GLuint tex, float x, float y, float w, float h, float rotation, unsigned int color)
    {
        sprites.push_back({ tex, {x, y}, {w, h}, rotation, color });
    }

    
    void Drawer::AddImage(GLuint tex, float x, float y, float w, float h, unsigned int color)
    {
        images.push_back({ tex, {x, y}, {w, h}, color });
    }
    */
    static bool comment = false;
    //void Drawer::Render(const Matrix<3>& cameraMatrix)

    void Drawer::Render(const Matrix<3>&)
    {

        static bool commentPrinted = false;
        static bool lastActiveState = false; // remember previous state

        // detect state change
        if (active != lastActiveState) {
            commentPrinted = false;   // reset on toggle
            lastActiveState = active;
        }

        if (!active) {
            //commentPrinted = false;
            if (!commentPrinted) {
                std::cout << "[DebugDraw] inactive\n";
                commentPrinted = !commentPrinted;
            }
            return;
        }

        // when active
        if (!commentPrinted) {
            std::cout << "[DebugDraw] rendering: "
                << rects.size() << " rects, "
                << circles.size() << " circles, "
                << lines.size() << " lines\n";
            commentPrinted = !commentPrinted;
        }

        glPushMatrix(); // save current transform
        //glMultMatrixf(cameraMatrix.data.data()); // Apply camera transform if needed

        // draw the rectangles
        for (auto& r : rects)
        {
            float R = ((r.color >> 24) & 0xFF) / 255.0f;
            float G = ((r.color >> 16) & 0xFF) / 255.0f;
            float B = ((r.color >> 8) & 0xFF) / 255.0f;
            float A = (r.color & 0xFF) / 255.0f;

            glColor4f(R, G, B, A);
            glBegin(GL_LINE_LOOP); // draw only the outline
            glVertex2f(r.pos.x(), r.pos.y()); // bottom left    
            glVertex2f(r.pos.x() + r.w, r.pos.y()); // bottom right
            glVertex2f(r.pos.x() + r.w, r.pos.y() + r.h); // top right
            glVertex2f(r.pos.x(), r.pos.y() + r.h); // top left
            glEnd();
        }

        // draw the circles
        for (auto& c : circles)
        {
            float R = ((c.color >> 24) & 0xFF) / 255.0f;
            float G = ((c.color >> 16) & 0xFF) / 255.0f;
            float B = ((c.color >> 8) & 0xFF) / 255.0f;
            float A = (c.color & 0xFF) / 255.0f;

            glColor4f(R, G, B, A);
            glBegin(GL_LINE_LOOP); // drawing outline only
            for (int i = 0; i < c.segments; i++)
            {
                float theta = 2.0f * 3.14159f * i / c.segments; // angle per segment
                glVertex2f(c.center.x() + c.radius * cosf(theta),
                    c.center.y() + c.radius * sinf(theta));
            }
            glEnd();
        }

        // draw the lines
        for (auto& l : lines)
        {
            float R = ((l.color >> 24) & 0xFF) / 255.0f;
            float G = ((l.color >> 16) & 0xFF) / 255.0f;
            float B = ((l.color >> 8) & 0xFF) / 255.0f;
            float A = (l.color & 0xFF) / 255.0f;

            glColor4f(R, G, B, A);
            glBegin(GL_LINES); // draw each pair as a line
            glVertex2f(l.start.x(), l.start.y());
            glVertex2f(l.end.x(), l.end.y());
            glEnd();
        }

        /*
        for (auto& s : sprites)
        {
            float R = ((s.color >> 24) & 0xFF) / 255.0f;
            float G = ((s.color >> 16) & 0xFF) / 255.0f;
            float B = ((s.color >> 8) & 0xFF) / 255.0f;
            float A = (s.color & 0xFF) / 255.0f;

            glColor4f(R, G, B, A);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, s.textureID);

            float cx = s.pos.x() + s.size.x() * 0.5f;
            float cy = s.pos.y() + s.size.y() * 0.5f;
            float cosA = std::cos(s.rotation);
            float sinA = std::sin(s.rotation);

            // Compute rotated corners
            float x0 = cx + (-s.size.x() * 0.5f) * cosA - (-s.size.y() * 0.5f) * sinA;
            float y0 = cy + (-s.size.x() * 0.5f) * sinA + (-s.size.y() * 0.5f) * cosA;
            float x1 = cx + (s.size.x() * 0.5f) * cosA - (-s.size.y() * 0.5f) * sinA;
            float y1 = cy + (s.size.x() * 0.5f) * sinA + (-s.size.y() * 0.5f) * cosA;
            float x2 = cx + (s.size.x() * 0.5f) * cosA - (s.size.y() * 0.5f) * sinA;
            float y2 = cy + (s.size.x() * 0.5f) * sinA + (s.size.y() * 0.5f) * cosA;
            float x3 = cx + (-s.size.x() * 0.5f) * cosA - (s.size.y() * 0.5f) * sinA;
            float y3 = cy + (-s.size.x() * 0.5f) * sinA + (s.size.y() * 0.5f) * cosA;

            glBegin(GL_TRIANGLES);
            // tri 1
            glTexCoord2f(0.f, 0.f); glVertex2f(x0, y0);
            glTexCoord2f(1.f, 0.f); glVertex2f(x1, y1);
            glTexCoord2f(1.f, 1.f); glVertex2f(x2, y2);
            // tri 2
            glTexCoord2f(0.f, 0.f); glVertex2f(x0, y0);
            glTexCoord2f(1.f, 1.f); glVertex2f(x2, y2);
            glTexCoord2f(0.f, 1.f); glVertex2f(x3, y3);
            glEnd();

            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
        }


        for (auto& img : images)
        {
            float R = ((img.color >> 24) & 0xFF) / 255.0f;
            float G = ((img.color >> 16) & 0xFF) / 255.0f;
            float B = ((img.color >> 8) & 0xFF) / 255.0f;
            float A = (img.color & 0xFF) / 255.0f;

            glColor4f(R, G, B, A);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, img.textureID);

            glBegin(GL_QUADS);
            glTexCoord2f(0.f, 0.f); glVertex2f(img.pos.x(), img.pos.y());
            glTexCoord2f(1.f, 0.f); glVertex2f(img.pos.x() + img.size.x(), img.pos.y());
            glTexCoord2f(1.f, 1.f); glVertex2f(img.pos.x() + img.size.x(), img.pos.y() + img.size.y());
            glTexCoord2f(0.f, 1.f); glVertex2f(img.pos.x(), img.pos.y() + img.size.y());
            glEnd();

            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
        }
        */

        //glPopAttrib(); // restore previous OpenGL settings
        glPopMatrix();  // restore transform
        Clear();    // clear all shapes for next frame
    }

    //removes all shapes
    void Drawer::Clear()
    {
        rects.clear();
        circles.clear();
        lines.clear();
    }

    void Drawer::Toggle(bool on)
    {
        active = on;  // turn debug mode on or off
    }
    

    bool Drawer::IsActive() const
    {
        return active; // return current debug mode state
    }
}
