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

struct Character {
    GLuint TextureID;     // ID of glyph texture
    glm::ivec2 Size;      // Size of glyph
    glm::ivec2 Bearing;   // Offset from baseline to left/top of glyph
    GLuint Advance;       // Offset to advance to next glyph
};

class Font : public Resource {
public:
    // Construct a Font resource with metadata; glyphs are loaded via LoadFont.
    Font(ResourceID id, const std::string& key, const std::string& filePath, unsigned int pixelSize);
    ~Font();

    // Upload glyphs and GPU resources for the requested size.
    bool LoadFont(const std::string& fontPath, unsigned int pixelSize);

    // Render helpers; unchanged
    void RenderText(Shader& shader, const std::string& text, float x, float y, float scale, glm::vec3 color);
    void Cleanup();

    unsigned int getPixelSize() const { return pixelSize; }

    // Expose read-only glyph data and GL handles so Renderer can draw text
    const std::map<GLchar, Character>& GetCharacters() const { return Characters; }
    GLuint GetVAO() const { return VAO; }
    GLuint GetVBO() const { return VBO; }

private:
    std::map<GLchar, Character> Characters{};
    GLuint VAO{0}, VBO{0};
    FT_Library ft{nullptr};
    FT_Face face{nullptr};
    unsigned int pixelSize{48};
};

#endif