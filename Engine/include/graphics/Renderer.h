// Renderer.h
#pragma once
#include "gl/glew.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h" 
#include "core/Logger.h"

extern Logger logger;

class Renderer {
public:
    //constructor for image rendering, need to provide vertex file AND fragment file
    Renderer(const char* vertex_path, const char* fragment_path);

    void init2D(int w, int h);
    void resize(int w, int h);
    void clear(float r, float g, float b);

    void tri(float x1, float y1, float x2, float y2, float x3, float y3);
    void rect(float x, float y, float w, float h);
    void line(float x1, float y1, float x2, float y2);
    void rectOutline(float x, float y, float w, float h);
    void circle(float cx, float cy, float radius, int segments = 32);

    void texturedTri(GLuint tex, float x, float y, float w, float h);

    //void drawAllSprites(Texture& tex);

   // Renderer.h
    void initAllSprites();

    void drawAllSprites(Texture& tex,
        const std::vector<int>& frontIdxs,
        bool behindPass,
        int viewportW, int viewportH); // wrapper

    void drawAllSprites(Texture& tex,
        const std::vector<int>& frontIdxs,
        bool behindPass,
        const float* scales,
        const float* rotations,
        int viewportW, int viewportH); // actual impl




    void background_render(GLuint texture_ID, const GLchar* uniform_VARNAME);

    // FBO setup and usage
    void initFBO(int width, int height);
    void bindFBO() const;
    void unbindFBO() const;
    GLuint getFBOTexture() const;   


    //// NEW: one function to draw bg + all shapes into the currently bound framebuffer
    //void drawViewportContent(int w, int h, Texture& tex);                    // overload 1
    //void drawViewportContent(int w, int h, GLuint texId, const char* name);  // overload 2

    void drawViewportShapes(int w, int h);
    struct sprite {
            int idx; 
            float x, y, w, h;
            float angle;
        };

private:
    GLuint VAO_arr = 0, VBO_arr = 0;
    GLuint viewportFBO = 0, viewportTexture = 0;
    int    fboWidth = 0, fboHeight = 0;

    Shader render_shader;
    
    std::vector<sprite> sprite_vector;

public:
    std::vector<sprite>& get_sprite_vector(); //get the entire sprite vector
};


// Simple helper to draw a sprite by texture index
static void drawSprite(Renderer& renderer, Texture& tex, int index,
    float x, float y, float w, float h)
{
    renderer.texturedTri(tex.get_textureID(static_cast<size_t>(index)), x, y, w, h);
}

