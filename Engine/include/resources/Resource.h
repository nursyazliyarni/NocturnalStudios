#pragma once

#include <string>
#include <numeric>

using ResourceID = unsigned;
constexpr ResourceID NEW_RESOURCE = std::numeric_limits<ResourceID>::max();
constexpr ResourceID INVALID_RESOURCE_ID = std::numeric_limits<ResourceID>::max();
constexpr ResourceID NO_RESOURCE_ID = std::numeric_limits<ResourceID>::max();
constexpr ResourceID NO_RESOURCE_SELECTED_ID = std::numeric_limits<ResourceID>::max();

class Resource {
public:
	Resource(ResourceID id, const std::string& key, const std::string& filePath);
	std::string const& getFilePath() const;
	void setFilePath(const std::string& filePath);

	std::string const& getKey() const;
	void setKey(const std::string& key);

	ResourceID const& getID() const;

	Resource(const Resource& other) = delete;
	Resource& operator=(const Resource& other) = delete;
	Resource(Resource&& other) = default;
	Resource& operator=(Resource&& other) = default;

protected:
	ResourceID id;
	std::string key;
	std::string filePath;
};