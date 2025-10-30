#pragma once

#include "Resource.h"

class PrefabFile : public Resource {
public:
	PrefabFile(ResourceID id, const std::string& key, const std::string& filePath);
};
