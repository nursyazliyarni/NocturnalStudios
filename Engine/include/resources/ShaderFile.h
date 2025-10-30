#pragma once

#include "Resource.h"

class ShaderFile : public Resource {
public:
	ShaderFile(ResourceID id, const std::string& key, const std::string& filePath);

	ShaderFile(ShaderFile const& other) = delete;
	ShaderFile& operator=(ShaderFile const& other) = delete;
	ShaderFile(ShaderFile&& other) = default;
	ShaderFile& operator=(ShaderFile&& other) = default;
};
