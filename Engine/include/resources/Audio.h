#pragma once

#include "../include/resources/ResourceManager.h"
#include <string>

class Audio : public ResourceManager {
public:
	Audio(const std::string& key, const std::string& filePath);
	Audio(Audio const& other) = delete;
	Audio& operator=(Audio const& other) = delete;
	Audio(Audio&& other) = default;
	Audio& operator=(Audio&& other) = default;
};