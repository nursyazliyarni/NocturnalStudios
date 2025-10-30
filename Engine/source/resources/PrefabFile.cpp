#include "resources/PrefabFile.h"
#include "core/Logger.h"

extern Logger logger;

PrefabFile::PrefabFile(ResourceID id, const std::string& key, const std::string& filePath)
	: Resource(id, key, filePath) {
	logger.log("PrefabFile resource created: " + key + " from " + filePath);
}
