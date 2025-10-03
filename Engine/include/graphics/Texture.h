/*!------------------------------------------------------------------------------------------------
	File: Texture.h
	Project: NocturnalStudio

	This file contains functions that loads the images from assets folder to be used for rendering
	our sprites.

	Primary Author:
		 Ang Yu Xuan (100%)
		-This file contains functions that loads the images from assets folder and free the textures when they
		are out of scope. Functions such as get image width and height is also available.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include <GL/glew.h>
#include <iostream>
#include <vector>
#include "core/Logger.h"

extern Logger logger;

class Texture {
public:
	Texture(std::vector<char const*>& image_path_v);
	~Texture();

	size_t get_totalTexture() const; //How many textures are loaded (used to size the scale array)
	const GLuint& get_textureID(size_t input_index) const;
	const int& get_texture_width(size_t input_index) const;
	const int& get_texture_height(size_t input_index) const;

private:
	struct Texture_Info {
		GLuint textureID_storage; //texture ID
		std::pair<int, int> image_dimension; //pair.first==width,pair.second==height
	};

	//use vector to store Texture_Info
	std::vector<Texture_Info> texture_info_vector;
};