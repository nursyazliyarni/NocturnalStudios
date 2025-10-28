#pragma once

#include "resources/Resource.h"
#include <string>

class Audio : public Resource {
public:
	Audio(ResourceID id, const std::string& key, const std::string& filePath);

	Audio(Audio const& other) = delete;
	Audio& operator=(Audio const& other) = delete;
	Audio(Audio&& other) = default;
	Audio& operator=(Audio&& other) = default;
};