#pragma once

#include "Resource.h"

class TextureFile : public Resource {
	public:
	TextureFile(ResourceID id, const std::string& key, const std::string& filePath);

	TextureFile(TextureFile const& other) = delete;
	TextureFile& operator=(TextureFile const& other) = delete;
	TextureFile(TextureFile&& other) = default;
	TextureFile& operator=(TextureFile&& other) = default;
};
