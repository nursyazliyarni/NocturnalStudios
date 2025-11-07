/*!------------------------------------------------------------------------------------------------
    File: Renderer.h
    Project: NocturnalStudio

    This file contains the declaration of the Renderer class for the engine�s 2D pipeline: background 
    quad setup(GL_TRIANGLES) + shader, FBO/viewport init & resize, immediate-mode debug primitives 
    (tri, rect, rectOutline, line, circle), and batched sprite rendering (textured quads with rotation, 
    scaling, and front/behind passes). It also exposes the FBO texture for UI display.

    Primary Author:
         Rishika Bimal Attodi (%)
        -Sprite system: batched textured quads, selection/front-vs-behind pass handling,
      per-sprite rotation & scaling (excluding background).

    Secondary Author:
         Name (%)
        -Brief

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// Renderer.h
#pragma once
#include "gl/glew.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "graphics/Shader.h"
#include "graphics/Texture.h" 
#include "core/Logger.h"
#include <glm/glm.hpp>  // for glm::vec3

#define UNREFERENCE_PARAMETER(P) (void)(P)

extern Logger logger;

// forward declaration to avoid header coupling
class Font;

class Renderer {
public:
    //constructor for image rendering, need to provide vertex file AND fragment file
    Renderer(std::vector<std::pair<std::string, std::string>> shader_filePATH_list);

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




    void background_render(GLuint texture_ID, const GLchar* uniform_VARNAME, unsigned int vector_index);
    void special_background_render(GLuint texture_ID, const GLchar* uniform_VARNAME, unsigned int vector_index);

    // FBO setup and usage
    void initFBO(int width, int height);
    void bindFBO() const;
    void unbindFBO() const;
    GLuint getFBOTexture() const;   

    void drawViewportShapes(int w, int h);

    // Text rendering (Renderer-controlled)
    void setTextShader(Shader& shader);
    void setTextProjection(int w, int h); // bottom-left origin for text
    void drawText(const Font& font, const std::string& text, float x, float y, float scale, const glm::vec3& color);

    struct sprite {
            int idx; 
            float x, y, w, h;
            float angle;
        };

	//draw a sprite using sub-UVs from a spritesheet
    void drawSpriteUV(GLuint tex, float x, float y, float w, float h,
        float u0, float v0, float u1, float v1);

private:
    GLuint VAO_arr = 0, VBO_arr = 0;
    GLuint viewportFBO = 0, viewportTexture = 0;
    int    fboWidth = 0, fboHeight = 0;

    Shader render_shader;

    // text state
    Shader* text_shader = nullptr;

    std::vector<sprite> sprite_vector;

public:
    std::vector<sprite>& get_sprite_vector(); //get the entire sprite vector
};


// Simple helper to draw a sprite by texture index
inline void drawSprite(Renderer& renderer, Texture& tex, int index,
    float x, float y, float w, float h)
{
    renderer.texturedTri(tex.get_textureID(static_cast<size_t>(index)), x, y, w, h);
}

