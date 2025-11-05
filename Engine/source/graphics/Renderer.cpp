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
#include "graphics/DebugDraw.h"

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

Renderer::Renderer(std::vector<std::pair<std::string, std::string>> shader_filePATH_list):render_shader(shader_filePATH_list) {
    //vertex position->where to render on screen(-1.0 to +1.0)
    //texture UV->how to render texture(0.0 to 1.0)
    float vertices[] = {
    //flip the texture UV to produce correct image
    //vertex positions(x,y)  texture UV   vertex positions(x,y) texture UV 
    -1.0f,  1.0f,            0.0f, 0.0f,  //top left            //bottom left
    -1.0f, -1.0f,            0.0f, 1.0f,  //bottom left         //top left
    1.0f,  1.0f,             1.0f, 0.0f,  //top right           //bottom right
    1.0f, -1.0f,             1.0f, 1.0f   //bottom right        //top right
    };

    //glGenVertexArrays returns the number of Vertex Array Objects(VAO) and store its unique ID in VAO_arr
    glGenVertexArrays(1, &VAO_arr);
    glBindVertexArray(VAO_arr); //glBindVertexArray binds the vertex array object with VAO_arr

    //glGenBuffers generates the number of Vertex Buffer Objects(VBO) and store its unique ID in VBO_arr
    glGenBuffers(1, &VBO_arr);
    //GL_ARRAY_BUFFER->vertex buffer that contains positions, colors, texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, VBO_arr); //glBindBuffer binds GL_ARRAY_BUFFER with VBO_arr
    //Assign vertex data stored in vertices to the currently bound vertex buffer(GL_ARRAY_BUFFER) 
    //the function allocates sizeof(vertices) to GPU
    //GL_STATIC_DRAW->The data store contents will be modified once and used many times and used for rendering
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Position attribute in background VERT file(location = 0)
    //The function will go to location 0 and read 2 float data type(8 bytes) starting from first element index postion
    //Then it will jump 4 * sizeof(float) to get to next vertex's position, GL_FALSE->DO NOT NORMALIZE
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); //how to read
    glEnableVertexAttribArray(0); //use location=0 for rendering
    //Texture coordinate attribute in background VERT file(location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1); //use location=1 for rendering
    glBindVertexArray(0); //unbinds the currently bound VAO
}

void Renderer::background_render(GLuint texture_ID, const GLchar* uniform_VARNAME, unsigned int vector_index)
{
    render_shader.user_Shader_program(vector_index); //use this shader program for rendering background image
    glActiveTexture(GL_TEXTURE0); //specifies which texture unit to make active
    glBindTexture(GL_TEXTURE_2D, texture_ID); //bind this current texture to texturing target
    //glGetUniformLocation takes in ID of shader program object AND uniform_VARNAME
    //to return the location of a uniform variable
    //glUniform1i takes in location of a uniform variable and 0(this means texture unit is GL_TEXTURE0)
    render_shader.SET_uniform_variable(uniform_VARNAME, 0, vector_index);

    glBindVertexArray(VAO_arr); //Bind Vertex Array Object to background mesh
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //Render rectangle(4 vertices = 2 triangles)
    glBindVertexArray(0); //unbinds the currently bound VAO
    glUseProgram(0); //unbind the currently active shader program and go back to fixed-function pipeline mode
}

void Renderer::special_background_render(GLuint texture_ID, const GLchar* uniform_VARNAME, unsigned int vector_index)
{
    render_shader.user_Shader_program(vector_index); //use this shader program for rendering background image
    glActiveTexture(GL_TEXTURE0); //specifies which texture unit to make active
    glBindTexture(GL_TEXTURE_2D, texture_ID); //bind this current texture to texturing target
    //glGetUniformLocation takes in ID of shader program object AND uniform_VARNAME
    //to return the location of a uniform variable
    //glUniform1i takes in location of a uniform variable and 0(this means texture unit is GL_TEXTURE0)
    render_shader.SET_uniform_variable(uniform_VARNAME, 0, vector_index);

    float time = (float)glfwGetTime();
    glUniform1f(glGetUniformLocation(render_shader.get_programID(vector_index), "time"), time);
    glUniform1f(glGetUniformLocation(render_shader.get_programID(vector_index), "parallaxFactor"), 1.0f);

    glBindVertexArray(VAO_arr); //Bind Vertex Array Object to background mesh
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //Render rectangle(4 vertices = 2 triangles)
    glBindVertexArray(0); //unbinds the currently bound VAO
    glUseProgram(0); //unbind the currently active shader program and go back to fixed-function pipeline mode
}

void Renderer::init2D(int w, int h) {
    resize(w, h);                 // set pixel-space projection

    // sensible defaults for 2D
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure we�re not in wireframe
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


void Renderer::texturedTri(GLuint tex, float x, float y, float w, float h) {
    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    // two triangles: (x,y)-(x+w,y)-(x+w,y+h) and (x,y)-(x+w,y+h)-(x,y+h)
    glBegin(GL_TRIANGLES);
    // tri 1
    glTexCoord2f(0.f, 0.f); glVertex2f(x, y);
    glTexCoord2f(1.f, 0.f); glVertex2f(x + w, y);
    glTexCoord2f(1.f, 1.f); glVertex2f(x + w, y + h);
    // tri 2
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

    // Delete old FBO/Texture if they exist
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

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind
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
    // make sure our 2D projection matches the target size
    resize(w, h);

    // 2D pass: fixed pipeline + no depth
    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);

    // RGB triangle (center/top)
    /*tri(w * 0.50f, h * 0.20f,
        w * 0.20f, h * 0.80f,
        w * 0.80f, h * 0.80f);*/

        // shrink factor (20%)
    float scale = 0.8f;
    float cx = w * 0.5f;
    float cy = h * 0.5f;

    tri(
        cx + (w * 0.50f - cx) * scale, cy + (h * 0.20f - cy) * scale,   // top
        cx + (w * 0.20f - cx) * scale, cy + (h * 0.80f - cy) * scale,   // bottom-left
        cx + (w * 0.80f - cx) * scale, cy + (h * 0.80f - cy) * scale    // bottom-right
    );


    // filled blue circle on the right
    {
        float r = h * 0.10f;
        float margin = w * 0.05f;
        float cx = w - (r + margin);
        if (cx < r) cx = r;
        float cy = h * 0.20f;

        glColor3f(0.f, 0.5f, 1.f);
        filledCircle(cx, cy, r, 64);
    }

    // white outline box (bottom-left)
    {
        float pad = w * 0.01f;
        float bw = w * 0.35f;
        float bh = h * 0.5f;
        rectOutline(pad, pad, bw, bh);
    }
}


//void Renderer::drawAllSprites(Texture& tex) {
//    struct QuadArgs { int idx; float x, y, w, h; };
//    const QuadArgs sprites[] = {
//        { 1,  60.f, 60.f, 128.f, 128.f }, // Circle
//        { 2, 220.f, 60.f, 128.f, 128.f }, // Square
//        { 3, 380.f, 60.f, 128.f, 128.f }, // placeholder cat
//        { 4, 540.f, 60.f, 128.f, 128.f }  // placeholder2 cat
//    };
//
//    glUseProgram(0);
//    glDisable(GL_DEPTH_TEST);
//
//    for (int i = 0; i < (int)(sizeof(sprites) / sizeof(sprites[0])); ++i) {
//        GLuint t = tex.get_textureID((size_t)sprites[i].idx);
//
//        glEnable(GL_TEXTURE_2D);
//        glBindTexture(GL_TEXTURE_2D, t);
//
//        glBegin(GL_QUADS);
//        glTexCoord2f(0.f, 0.f); glVertex2f(sprites[i].x, sprites[i].y);
//        glTexCoord2f(1.f, 0.f); glVertex2f(sprites[i].x + sprites[i].w, sprites[i].y);
//        glTexCoord2f(1.f, 1.f); glVertex2f(sprites[i].x + sprites[i].w, sprites[i].y + sprites[i].h);
//        glTexCoord2f(0.f, 1.f); glVertex2f(sprites[i].x, sprites[i].y + sprites[i].h);
//        glEnd();
//
//        glBindTexture(GL_TEXTURE_2D, 0);
//        glDisable(GL_TEXTURE_2D);
//    }
//}


void Renderer::drawAllSprites(Texture& tex,
    const std::vector<int>& frontIdxs,
    bool behindPass,
    int viewportW, int viewportH)
{
    drawAllSprites(tex, frontIdxs, behindPass, /*scales*/ nullptr, /*rotations*/ nullptr, viewportW, viewportH);
}

void Renderer::initAllSprites() {
                      //int idx;|  float x, y, w, h;  |float scale_value;
    sprite_vector.push_back({ 1, 0.01f, 0.10f, 0.15f, 0.25f, 0.0f }); // drink (5% from left, 10% from top, 10% width, 15% height)
    sprite_vector.push_back({ 2, 0.15f, 0.10f, 0.15f, 0.25f, 0.0f }); // cupcake
    sprite_vector.push_back({ 3, 0.40f, 0.35f, 0.25f, 0.30f, 0.0f }); // Cat A
    sprite_vector.push_back({ 4, 0.70f, 0.25f, 0.25f, 0.30f, 0.0f }); // Cat B
    //sprite_vector.push_back({ 5, 0.40f, 0.35f, 0.25f, 0.30f, 0.0f }); //cat A ate
    //sprite_vector.push_back({ 6, 0.70f, 0.25f, 0.25f, 0.30f, 0.0f }); //cat B ate

}

void Renderer::drawAllSprites(Texture& tex,
    const std::vector<int>& frontIdxs,
    bool behindPass,
    const float* scales,
    const float* rotations,
    int viewportW, int viewportH)
{
    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);

    const size_t texCount = tex.get_totalTexture(); // includes bg at index 0

    for (int i = 0; i<sprite_vector.size(); ++i) {
        bool isFront = std::find(frontIdxs.begin(), frontIdxs.end(), sprite_vector[i].idx) != frontIdxs.end();

        // pass filter
        if (behindPass && isFront)   continue; // behind pass: skip all �front� sprites
        if (!behindPass && !isFront) continue; // front pass: draw ONLY �front� sprites
        float s = 1.0f;// SCALING
        float angle = 0.0f;// ROTATION 

        // Convert normalized to pixels
        float x = sprite_vector[i].x * viewportW;
        float y = sprite_vector[i].y * viewportH;
        float w = sprite_vector[i].w * viewportW * s;
        float h = sprite_vector[i].h * viewportH * s;

        // Find center
        float cx = x + w * 0.5f;
        float cy = y + h * 0.5f;

        // Compute rotated corners
        float cosA = std::cos(sprite_vector[i].angle);
        float sinA = std::sin(sprite_vector[i].angle);

        // Helper to rotate each corner
        float x0, y0, x1, y1, x2, y2, x3, y3;

        // top-left
        float dx = x - cx;
        float dy = y - cy;
        x0 = cx + dx * cosA - dy * sinA;
        y0 = cy + dx * sinA + dy * cosA;

        // top-right
        dx = (x + w) - cx;
        dy = y - cy;
        x1 = cx + dx * cosA - dy * sinA;
        y1 = cy + dx * sinA + dy * cosA;

        // bottom-right
        dx = (x + w) - cx;
        dy = (y + h) - cy;
        x2 = cx + dx * cosA - dy * sinA;
        y2 = cy + dx * sinA + dy * cosA;

        // bottom-left
        dx = x - cx;
        dy = (y + h) - cy;
        x3 = cx + dx * cosA - dy * sinA;
        y3 = cy + dx * sinA + dy * cosA;

        GLuint t = tex.get_textureID((size_t)sprite_vector[i].idx);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, t);

        //// two triangles 
        //glBegin(GL_TRIANGLES);
        //// tri 1
        //glTexCoord2f(0.f, 0.f); glVertex2f(x, y);
        //glTexCoord2f(1.f, 0.f); glVertex2f(x + w, y);
        //glTexCoord2f(1.f, 1.f); glVertex2f(x + w, y + h);
        //// tri 2
        //glTexCoord2f(0.f, 0.f); glVertex2f(x, y);
        //glTexCoord2f(1.f, 1.f); glVertex2f(x + w, y + h);
        //glTexCoord2f(0.f, 1.f); glVertex2f(x, y + h);
        //glEnd();

        // two triangles using rotated corners
        glBegin(GL_TRIANGLES);
        //// tri 1
        //glTexCoord2f(0.f, 0.f); glVertex2f(sprite_vector[i].x, sprite_vector[i].y);
        //glTexCoord2f(1.f, 0.f); glVertex2f(sprite_vector[i].x + sprite_vector[i].w, sprite_vector[i].y);
        //glTexCoord2f(1.f, 1.f); glVertex2f(sprite_vector[i].x + sprite_vector[i].w, sprite_vector[i].y + sprite_vector[i].h);

        //// tri 2
        //glTexCoord2f(0.f, 0.f); glVertex2f(x0, y0);
        //glTexCoord2f(1.f, 1.f); glVertex2f(x2, y2);
        //glTexCoord2f(0.f, 1.f); glVertex2f(x3, y3);
        //glEnd();

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

        // DEBUGDRAW: visualize AABB collider ---
       // draw collider outline when debug is active
        if (Debug::Drawer::Instance().IsActive())
        {
            // cyan box outline
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


