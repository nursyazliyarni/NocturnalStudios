#include "resources/ShaderFile.h"
#include "core/Logger.h"

extern Logger logger;

ShaderFile::ShaderFile(ResourceID id, const std::string& key, const std::string& filePath)
	: Resource(id, key, filePath) {
	logger.log("ShaderFile resource created: " + key + " from " + filePath);
}