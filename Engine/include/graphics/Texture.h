#include <GL/glew.h>
#include <iostream>
#include <vector>

//void glGenFramebuffers(GLsizei n,GLuint* ids)
	//returns n framebuffer object names in ids.
	//glGenFramebuffers(1, &FramebufferName);

	//void glBindFramebuffer(GLenum target,GLuint framebuffer);
	//binds the framebuffer object with name framebuffer to the framebuffer target specified by target
	//glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

class Texture {
public:
	Texture(std::vector<char const*>& image_path_v);
	~Texture();

private:
	//use vector to store texture IDs for multiple images
	std::vector<GLuint> textureID_storage;

};