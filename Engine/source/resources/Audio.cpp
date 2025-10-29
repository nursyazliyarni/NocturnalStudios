#include <iostream>

#include "resources/Audio.h"
#include "core/Logger.h"
extern Logger logger;

Audio::Audio(ResourceID id, const std::string& key, const std::string& filePath)
	: Resource(id, key, filePath) {
	logger.log("Audio resource created: " + key + " from " + filePath);
}