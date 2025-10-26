/* !------------------------------------------------------------------------------------------------
	File: Texture.cpp
	Project : NocturnalStudio

		This file contains functions that loads the images from assets folder to be used for rendering
	    our sprites.

	Ang Yu Xuan (100%)
		-This file contains functions that loads the images from assets folder and free the textures when they
		are out of scope. Functions such as get image width and height is also available.

All content(c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
* ------------------------------------------------------------------------------------------------ */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "graphics/Texture.h"

Texture::Texture(std::vector<char const*> &image_path_v):texture_info_vector(image_path_v.size()) {
	int num_components;
	for (size_t index{ 0 }; index < image_path_v.size(); index++) {
		//image in pixel, width, height num_components->image components for image file,
		unsigned char* image_data{ stbi_load(image_path_v[index], &texture_info_vector[index].image_dimension.first, 
			&texture_info_vector[index].image_dimension.second, &num_components, 0)};
		if (!image_data) { //if image loading fails
			logger.error("Failed to load texture "+ std::string(image_path_v[index])+": "+
				        ((stbi_failure_reason())));
			continue; //skip this iteration
		}
		logger.log("Texture "+std::string(image_path_v[index])+" loads successfully");

		//GLuint/Glenum->unsigned int, Glsizei->int
		//generate 1 unique textureID at a time
		glGenTextures(1, &texture_info_vector[index].textureID_storage); //GLsizei, GLuint*
		glBindTexture(GL_TEXTURE_2D, texture_info_vector[index].textureID_storage); //GLenum texture_target, Gluint

		//GLenum texture_target,level of mipmap reduction image{0},number of color components in the texture,
		//image_dimension,border(must be 0),format of the pixel data,data type of the pixel data,pointer to the image_data
		//Pick correct format depending on how many channels stb_image gave
		GLenum format = GL_RGB;
		if (num_components == 4)      format = GL_RGBA;  // has alpha
		else if (num_components == 3) format = GL_RGB;   // standard RGB
		else if (num_components == 2) format = GL_RG;    // Red + Green
		else if (num_components == 1) format = GL_RED;   // grayscale

		//Prevent row padding issues for images whose width is not multiple of 4
		//GL_UNPACK_ALIGNMENT sets the byte alignment requirement for the start of each pixel row in memory 
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, format, texture_info_vector[index].image_dimension.first, 
			texture_info_vector[index].image_dimension.second, 0, format, GL_UNSIGNED_BYTE, image_data);

		//Set texture wrapping AND filtering options
		//S->x-axis,T->y->axis,GL_CLAMP_TO_EDGE->clamp coordinates of texture between 0 and 1
		//GL_LINEAR->the weighted average of the four texture elements(form the 2x2 square) that are closest to 
		//the specified texture coordinates
		//GL_TEXTURE_MIN_FILTER->handle the minification of pixel
		//GL_TEXTURE_MAG_FILTER->handle the magnification of pixel 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(image_data); //free the loaded image
	}
}

Texture::~Texture() {
	if (!texture_info_vector.empty()) { //if vector.size()!=0
		for (size_t index{ 0 }; index < get_totalTexture(); index++) {
			//delete 1 unique textureID at a time
			glDeleteTextures(1, &texture_info_vector[index].textureID_storage);
		}
	}
}

//How many textures are loaded (used to size the scale array)
size_t Texture::get_totalTexture() const {
	return texture_info_vector.size();
}

const GLuint& Texture::get_textureID(size_t input_index) const {
	if (input_index<texture_info_vector.size())
	return texture_info_vector[input_index].textureID_storage;
}

const int& Texture::get_texture_width(size_t input_index) const {
	if (input_index < texture_info_vector.size())
	return texture_info_vector[input_index].image_dimension.first;
}

const int& Texture::get_texture_height(size_t input_index) const {
	if (input_index < texture_info_vector.size())
	return texture_info_vector[input_index].image_dimension.second;
}
