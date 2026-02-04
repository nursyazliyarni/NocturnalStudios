/*!------------------------------------------------------------------------------------------------
    File: TextRendering.cpp
    Project: NocturnalStudio

	Brief:
		This file render text using texture atlas and glm
	
    Primary Author:
         LAU XIANG YANG (95%)
		- Created the text rendering class and its function specification.
    Secondary Author:
         Nur Syazliyarni (5%)
        - Added Text component rendering support.
        
    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include "Graphics/TextRendering.h"
#include "ResourceManager/ResourceManager.h"

namespace NocturnalEngine {

    /*!*************************************************************************
    Init loop for render font
    ****************************************************************************/
	void Font::Init()
	{
		TextShader = ResourceManager::LoadShader("text", "Assets/Shaders/text.shader");;
		glGenVertexArrays(1, &mVao);
		glGenBuffers(1, &mVbo);
		glBindVertexArray(mVao);
		glBindBuffer(GL_ARRAY_BUFFER, mVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

    /*!*************************************************************************
    Load font
    ****************************************************************************/
	void Font::Load(const std::string& fontpath, unsigned int fontsize)
	{

        Characters.clear();
        // then initialize and load the FreeType library
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) // all functions return a value different than 0 whenever an error occurred
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        // load font as face
        FT_Face face;
        if (FT_New_Face(ft, fontpath.c_str(), 0, &face))
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, fontsize);

        float ladv = (face->size->metrics.height >> 6);
        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // then for the first 128 ASCII characters, pre-load/compile their characters and store them
        for (GLubyte c = 0; c < 128; c++) // lol see what I did there 
        {
            // load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
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
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x),
                ladv
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        // destroy FreeType once we're finished
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
	}

    /*!*************************************************************************
    Render font
    ****************************************************************************/
    void Font::RenderText(const std::string& text, glm::vec2 position, int line_height, float scale, Camera2D& camera, glm::vec3 color,float rot)
	{
        // activate corresponding render state	
        TextShader->Bind();
        TextShader->SetUniform("u_ViewProjection", camera.GetViewProjectionMatrix());
        
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(mVao);

        rot = glm::radians(rot);
        glm::vec2 pivot = position;   // rotate around start of text

        glm::vec3 charColor = color; // set to default color

        // iterate through all characters
        std::string::const_iterator c;
        float startX = position.x;
        for (c = text.begin(); c != text.end(); c++)
        {
            if (*c == '#') { charColor = { 225.0f, 0.2f, 0.2f }; continue; }     // red
            else if (*c == '@') { charColor = { 0.2f, 1.0f, 0.2f }; continue; }// green
            else if (*c == '$') { charColor = color; continue; }//back to defau;t

            if (*c == '\n') {
                position.x = startX;
                position.y -= scale* Characters[*c].LineAdvance;
                continue;
            }

            Character ch = Characters[*c];

            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;

            // --- NEW: build quad first (unrotated) ---
            float xpos = position.x + ch.Bearing.x * scale;
            float ypos = position.y - (ch.Size.y - ch.Bearing.y) * scale;

            glm::vec2 p1 = { xpos,     ypos + h }; // top-left
            glm::vec2 p2 = { xpos,     ypos }; // bottom-left
            glm::vec2 p3 = { xpos + w, ypos }; // bottom-right
            glm::vec2 p4 = { xpos + w, ypos + h }; // top-right

            // --- NEW: rotate around text origin ---
            p1 = RotatePoint(p1, pivot, rot);
            p2 = RotatePoint(p2, pivot, rot);
            p3 = RotatePoint(p3, pivot, rot);
            p4 = RotatePoint(p4, pivot, rot);

            // --- NEW: rotated vertices ---
            float vertices[6][4] = {
                { p1.x, p1.y, 0.0f, 0.0f },
                { p2.x, p2.y, 0.0f, 1.0f },
                { p3.x, p3.y, 1.0f, 1.0f },

                { p1.x, p1.y, 0.0f, 0.0f },
                { p3.x, p3.y, 1.0f, 1.0f },
                { p4.x, p4.y, 1.0f, 0.0f }
            };
            TextShader->SetUniform("textColor", charColor);
            // render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            // update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, mVbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph
            position.x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        
	}
}
