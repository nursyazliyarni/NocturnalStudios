/*!------------------------------------------------------------------------------------------------
    File: Font.h
    Project: NocturnalStudio

    This file contains Font Character structure and related function declarations.

    Primary Author:
         Chang Qi Xuan Celest
        - Created Character struct and font related class function declarations.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#ifndef FONT_H
#define FONT_H

#include "../freetype/include/ft2build.h"
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <map>
#include <string>
#include <iostream>

#include "graphics/Shader.h"
#include "Resource.h"
#include "core/Logger.h"
extern Logger logger;


/**
* @struct Character
* @brief Stores glyph-specific data for a single character.
* Each glyph is uploaded as an OpenGL texture and includes metrics
* for rendering text strings accurately.
*/
struct Character {
    GLuint TextureID;     // ID of glyph texture
    glm::ivec2 Size;      // Size of glyph
    glm::ivec2 Bearing;   // Offset from baseline to left/top of glyph
    GLuint Advance;       // Offset to advance to next glyph
};

/**
* @class Font
* @brief Represents a font resource capable of rendering text using FreeType and OpenGL.
*
* The Font class loads a TrueType/OpenType font file, extracts glyphs using FreeType,
* uploads them as textures, and provides rendering functionality for text strings.
*
* @note Font resources are managed by ResourceManager and used by Renderer for text rendering.
*/
class Font : public Resource {
public:
    /**
    * @brief Constructs a Font resource object.
    * @param id Unique resource ID.
    * @param key Resource key name.
    * @param filePath Path to the font file.
    * @param pixelSize Desired pixel height for glyph rendering.
    */
    Font(ResourceID id, const std::string& key, const std::string& filePath, unsigned int pixelSize);

    /** @brief Destructor that frees all FreeType and GPU resources. */
    ~Font();

    /**
    * @brief Loads the font and uploads glyphs to the GPU.
    * @param fontPath File path to the `.ttf` or `.otf` font file.
    * @param pixelSize Pixel height for each glyph.
    * @return True if the font was successfully loaded, else false.
    */
    bool LoadFont(const std::string& fontPath, unsigned int pixelSize);

    /**
    * @brief Renders a text string on screen using the specified shader.
    * @param shader Shader used for text rendering.
    * @param text Text string to render.
    * @param x X-coordinate in screen space.
    * @param y Y-coordinate in screen space.
    * @param scale Text scaling factor.
    * @param color RGB color of the text.
    */
    void RenderText(Shader& shader, const std::string& text, float x, float y, float scale, glm::vec3 color);

    /** @brief Cleans up FreeType face and library objects. */
    void Cleanup();

    /** @brief Returns the pixel size used when loading the font. */
    unsigned int getPixelSize() const { return pixelSize; }

    /** @brief Returns a read-only reference to all loaded glyphs. */
    const std::map<GLchar, Character>& GetCharacters() const { return Characters; }

    /** @brief Returns the VAO handle used for text rendering. */
    GLuint GetVAO() const { return VAO; }
    /** @brief Returns the VBO handle used for text rendering. */
    GLuint GetVBO() const { return VBO; }

private:
    std::map<GLchar, Character> Characters{}; //Glyth data for ASCII range 0-127
    GLuint VAO{0}, VBO{0}; //OpenGL Vertex Array Object and Vertex Buffer Object for text rendering 
	FT_Library ft{ nullptr }; //FreeType Library handle
	FT_Face face{ nullptr }; //FreeType Font Face handle
	unsigned int pixelSize{ 48 }; //Font pixel size
};

#endif