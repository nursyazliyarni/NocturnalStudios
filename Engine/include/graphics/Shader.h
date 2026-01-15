/*!------------------------------------------------------------------------------------------------
    File: Shader.h
    Project: NocturnalStudio

	Brief:
		This file contain the declaration for Shader which contains all necessary functions
		for the engine.
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Shader.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>
namespace NocturnalEngine {

	class Shader
	{
	public:
		Shader(const std::string& filepath);
		Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		~Shader();

		void Bind() const;
		void Unbind() const;

		//function overload 
		void SetUniform(const std::string& name, int value);
		void SetUniform(const std::string& name, int* value, unsigned int count);
		void SetUniform(const std::string& name, float value);
		void SetUniform(const std::string& name, const glm::vec2& value);
		void SetUniform(const std::string& name, const glm::vec3& value);
		void SetUniform(const std::string& name, const glm::vec4& value);
		void SetUniform(const std::string& name, const glm::mat3& value);
		void SetUniform(const std::string& name, const glm::mat4& value);
		void SetUniform(const std::string& name, bool value);

	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> Processing(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
	private:

		unsigned int mRendererID;
		std::string mName;
	};
}