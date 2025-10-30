#include "resources/TextureFile.h"
#include "core/Logger.h"

extern Logger logger;

TextureFile::TextureFile(ResourceID id, const std::string& key, const std::string& filePath)
	: Resource(id, key, filePath) {
	logger.log("TextureFile resource created: " + key + " from " + filePath);
}