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
		 Chang Qi Xuan Celest
		-Function to draw text controlled by Renderer class.

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
    void rect(float x, float y, float w, float h, float r, float g, float b);
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

    //------------Text rendering (Renderer-controlled)--------------
    /**
    * @brief Sets the shader used for text rendering.
    * This function assigns a dedicated shader to handle all text rendering
    * operations. It also binds the shader once to initialize the sampler uniform
    * used by glyph textures.
    * @param shader Reference to the shader that will be used for text rendering.
    * @see Renderer::setTextProjection()
    * @see Renderer::drawText()
    */
    void setTextShader(Shader& shader);

    /**
    * @brief Sets up an orthographic projection for text rendering.
    * Configures the projection matrix for text drawing in screen space with a
    * bottom-left origin. This ensures that text coordinates match the viewport
    * dimensions used for 2D UI or overlay rendering.
    * @param w Width of the viewport or target surface in pixels.
    * @param h Height of the viewport or target surface in pixels.
    * @note Must be called whenever the window or framebuffer is resized.
    * @see Renderer::setTextShader()
    */
    void setTextProjection(int w, int h); // bottom-left origin for text

    /**
    * @brief Draws text on screen using a loaded Font resource.
    * Renders the given text string at the specified screen-space position,
    * scaling, and color. This function iterates through glyph data stored
    * in the Font object and issues OpenGL draw calls for each character.
    * @param font The Font resource containing glyph textures and metrics.
    * @param text The string to render (ASCII supported by default).
    * @param x Horizontal position of the text (in pixels, bottom-left origin).
    * @param y Vertical position of the text (in pixels, bottom-left origin).
    * @param scale Scaling factor applied to the rendered text.
    * @param color RGB color of the text as a glm::vec3.
    * @note Requires a text shader set via `setTextShader()`.
    * @see Renderer::setTextShader()
    * @see Font::GetCharacters()
    */
    void drawText(const Font& font, const std::string& text, float x, float y, float scale, const glm::vec3& color);

    struct sprite {
            int idx; 
            float x, y, w, h;
            float angle;
        };

    /**
    * @brief Draws a textured rectangle (sprite) with custom UV coordinates.
    * This function renders a 2D sprite at the specified position and size,
    * using a portion of the texture defined by UV coordinates. It uses
    * immediate-mode OpenGL (`glBegin/glEnd`) for simple sprite rendering.
    * @param tex OpenGL texture ID to render.
    * @param x X position of the sprite's bottom-left corner (screen space).
    * @param y Y position of the sprite's bottom-left corner (screen space).
    * @param w Width of the sprite in pixels.
    * @param h Height of the sprite in pixels.
    * @param u0 Left (minimum) U coordinate in texture space (0.0–1.0).
    * @param v0 Bottom (minimum) V coordinate in texture space (0.0–1.0).
    * @param u1 Right (maximum) U coordinate in texture space (0.0–1.0).
    * @param v1 Top (maximum) V coordinate in texture space (0.0–1.0).
    * @note Depth testing is disabled for this draw call.
    *       Immediate-mode OpenGL is used, which is suitable for simple or debug rendering
    *       but not recommended for performance-critical production code.
    * @see Renderer::drawText() for text rendering using sprites/glyphs.
    */
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

