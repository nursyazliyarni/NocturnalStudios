/*!------------------------------------------------------------------------------------------------
    File: Font.cpp
    Project: NocturnalStudio

	This file contains Font functions definitions for font loading, rendering and cleanup.

    Primary Author:
         Chang Qi Xuan Celest
        - Created Font function definitions.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "resources/Font.h"

/**
* @brief Font constructor.
* Logs font resource creation and stores metadata.
*/
Font::Font(ResourceID id, const std::string& key, const std::string& filePath, unsigned int pixelVal)
    : Resource(id, key, filePath), pixelSize(pixelVal) {
    logger.log("Font resource created: " + key + " from " + filePath);
}

/**
* @brief Loads font data from disk and prepares glyph textures.
* Initializes FreeType, loads the specified font face, and uploads
* ASCII glyphs (0–127) as individual textures. Also sets up OpenGL buffers
* for dynamic text rendering.
* @param fontPath Path to the font file (.ttf or .otf).
* @param pixelVal Target pixel height for glyphs.
* @return True on success, false on failure.
*/
bool Font::LoadFont(const std::string& fontPath, unsigned int pixelVal) {
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR: Could not initialize FreeType Library!" << std::endl;
        return false;
    }

    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "ERROR: Failed to load font: " << fontPath << std::endl;
        return false;
    }

    this->pixelSize = pixelVal;

    FT_Set_Pixel_Sizes(face, 0, pixelVal);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (GLubyte c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR: Failed to load Glyph: " << (int)c << std::endl;
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    logger.log("Font loaded: " + key + " from " + fontPath + " at size " + std::to_string(pixelSize) + "px");
    return true;
}

/**
* @brief Renders a text string using the loaded glyph textures.
* Iterates through each character, updates vertex positions,
* and draws textured quads using OpenGL.
* @param shader The shader used for text rendering.
* @param text The UTF-8 string to render (ASCII supported by default).
* @param x Horizontal starting position in screen space.
* @param y Vertical starting position in screen space.
* @param scale Scaling factor applied to text size.
* @param color Text color (RGB).
*/
void Font::RenderText(Shader& shader, const std::string& text, float x, float y, float scale, glm::vec3 color) {
    shader.user_Shader_program(0);
    shader.SET_uniform_variable("textColor", color.x, color.y, color.z);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (auto c : text) {
        Character ch = Characters.at(c);

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
* @brief Releases FreeType and OpenGL resources used by the font.
*/
void Font::Cleanup() {
    if (face) {
        FT_Done_Face(face);
        face = nullptr;
    }
    if (ft) {
        FT_Done_FreeType(ft);
        ft = nullptr;
    }
}

/**
* @brief Destructor that calls Cleanup() to release all resources.
*/
Font::~Font() {
    Cleanup();
}
