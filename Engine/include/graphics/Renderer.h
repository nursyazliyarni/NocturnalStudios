// Renderer.h
#pragma once
#include "gl/glew.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"   // <-- add this
#include "core/Logger.h"

extern Logger logger;

class Renderer {
public:
    //constructor for image rendering, need to provide vertex file AND fragment file
    Renderer(const char* vertex_path, const char* fragment_path);
public:
    void init2D(int w, int h);
    void resize(int w, int h);
    void clear(float r, float g, float b);

    void tri(float x1, float y1, float x2, float y2, float x3, float y3);
    void rect(float x, float y, float w, float h);
    void line(float x1, float y1, float x2, float y2);
    void rectOutline(float x, float y, float w, float h);
    void circle(float cx, float cy, float radius, int segments = 32);

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


private:
    GLuint VAO_arr = 0, VBO_arr = 0;
    GLuint viewportFBO = 0, viewportTexture = 0;
    int    fboWidth = 0, fboHeight = 0;


    Shader render_shader;
};
