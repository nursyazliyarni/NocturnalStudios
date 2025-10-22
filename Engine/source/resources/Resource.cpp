#include "../include/resources/Resource.h"

Resource::Resource(ResourceID id, const std::string& key, const std::string& path) :
	id{ id }, key{ key }, filePath{ path } {}

const std::string& Resource::getKey() const {
	return key;
}

void Resource::setKey(const std::string& val) {
	key = val;
}

ResourceID const& Resource::getID() const {
	return id;
}

const std::string& Resource::getFilePath() const {
	return filePath;
}

void Resource::setFilePath(const std::string& val) {
	filePath = val;
}
