#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "graphics/Texture.h"

Texture::Texture(std::vector<char const*> &image_path_v):textureID_storage(image_path_v.size()) {
	stbi_set_flip_vertically_on_load(true);
	int image_width, image_height, num_components;
	for (size_t index{ 0 }; index < image_path_v.size(); index++) {
		//image in pixel, num_components->image components for image file,
		unsigned char* image_data{ stbi_load(image_path_v[index], &image_width, &image_height, &num_components, 0)};
		if (!image_data) { //if image loading fails
			std::cout << "Failed to load texture: " << stbi_failure_reason() << "\n";
			continue; //skip this iteration
		}

		//GLuint/Glenum->unsigned int, Glsizei->int
		glGenTextures(textureID_storage.size(), &textureID_storage[index]); //GLsizei, GLuint*
		glBindTexture(GL_TEXTURE_2D, textureID_storage[index]); //GLenum texture_target, Gluint
		//GLenum texture_target,level of mipmap reduction image{0},image_dimension,border(must be 0), 
		//format of the pixel data,data type of the pixel data,pointer to the image_data
		glTexImage2D(GL_TEXTURE_2D, 0, num_components, image_width, image_height, 0, num_components, GL_UNSIGNED_BYTE, 
			         image_data);

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
		std::cout << "YES\n";
	}
}

Texture::~Texture() {
	if (!textureID_storage.empty()) { //if vector.size()!=0
		glDeleteTextures(textureID_storage.size(), textureID_storage.data()); //delete all textures here

		std::cout << "NO\n";
	}
}
