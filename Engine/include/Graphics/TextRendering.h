/*!------------------------------------------------------------------------------------------------
    File: TextRendering.h
    Project: NocturnalStudio

	Brief:
		This file header contain all the necessary declaration for
		TextRendering
		
    Primary Author:
         LAU XIANG YANG (90%)
		- Created the text rendering class and its function specification.
    Secondary Author:
         Nur Syazliyarni (5%)
        - Added Text component rendering support.
        Ang Yu Xuan (5%)
        - Modify RenderText function to make it accept newline character and - sign

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include <glm/glm.hpp>
#include "Texture.h"
#include "Shader.h"
#include "Camera2D.h"
namespace NocturnalEngine {
	// Holds all state information relevant to a character as loaded using FreeType
	struct Character
	{
		unsigned int TextureID; //ID handler of glyph texture
		glm::ivec2 Size;		//size of glyph
		glm::ivec2 Bearing;		//offset from baseline to left/top of glyph
		unsigned int Advance;	//horizontal offset to next glyph
		float LineAdvance;
	};
	// A renderer class for rendering text displayed by a font loaded using the 
	// FreeType library. A single font is loaded, processed into a list of Character
	// items for later rendering.
	class Font
	{
	public:
		std::map<char, Character> Characters;
		
		Font() = default;
		void Init(); // load the path of the font we taking

		void Load(const std::string& fontpath, unsigned int fontsize = 50);

		void RenderText(const std::string& text, glm::vec2 position, int line_height, float scale, Camera2D& camera, glm::vec3 color = glm::vec3(1.0f),float rot=0.f);

	private:
		MultiRefs<Shader> TextShader;
		unsigned int mVao{ 0 }, mVbo{ 0 };

		glm::vec2 RotatePoint(glm::vec2 p, glm::vec2 pivot, float rad)
		{
			float s = sin(rad);
			float c = cos(rad);

			// move to origin
			p -= pivot;

			// rotate
			float xnew = p.x * c - p.y * s;
			float ynew = p.x * s + p.y * c;

			// move back
			p.x = xnew + pivot.x;
			p.y = ynew + pivot.y;

			return p;
		}
		
	};
}