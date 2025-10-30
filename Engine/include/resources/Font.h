#ifndef FONT_H
#define FONT_H

#include "../freetype/include/ft2build.h"
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <map>
#include <string>
#include <iostream>

#include "graphics/Shader.h"  // Your existing shader system


struct Character {
    GLuint TextureID;     // ID of glyph texture
    glm::ivec2 Size;      // Size of glyph
    glm::ivec2 Bearing;   // Offset from baseline to left/top of glyph
    GLuint Advance;       // Offset to advance to next glyph
};

class Font {
public:
    Font() = default;
    ~Font();

    bool LoadFont(const std::string& fontPath, unsigned int pixelSize);
    void RenderText(Shader& shader, const std::string& text, float x, float y, float scale, glm::vec3 color);
    void Cleanup();

private:
    std::map<GLchar, Character> Characters;
    GLuint VAO, VBO;
    FT_Library ft;
    FT_Face face;
};

#endif