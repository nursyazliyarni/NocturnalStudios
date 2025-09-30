#pragma once
#include <GL/glew.h>
#include <iostream>
#include <vector>

class Texture {
public:
	Texture(std::vector<char const*>& image_path_v);
	~Texture();

	GLuint get_textureID(size_t input_index);


private:
	//use vector to store texture IDs for multiple images
	std::vector<GLuint> textureID_storage;

};