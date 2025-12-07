/*!------------------------------------------------------------------------------------------------
	File: Texture.h
	Project: NocturnalStudio

	This file contains ...

	Primary Author:
		 Name (%)
		-Brief

	Secondary Author:
		 Name (%)
		-Brief

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

	//size_t get_totalTexture() const; //How many textures are loaded (used to size the scale array)
	//const GLuint& get_textureID(size_t input_index) const;
	//const int& get_texture_width(size_t input_index) const;
	//const int& get_texture_height(size_t input_index) const;

	size_t get_totalTexture() const;
	GLuint get_textureID(size_t) const;     
	int get_texture_width(size_t) const; 
	int get_texture_height(size_t) const;

private:
	struct Texture_Info {
		GLuint textureID_storage; //texture ID
		std::pair<int, int> image_dimension; //pair.first==width,pair.second==height
	};

	//use vector to store Texture_Info
	std::vector<Texture_Info> texture_info_vector;
};