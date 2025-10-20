/*!------------------------------------------------------------------------------------------------
	File: ECS.cpp
	Project: NocturnalStudio

	This file contains the implementation of non templated member function of the ECS class.

	Primary Author:
		Lau Xiang Yang (100%)
		- Created the file and implemented the ECS class member functions


	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "../include/ECS/ECS.h"
#include "../include/core/Logger.h"

extern Logger logger;

//-------------------------- ECS Class Implementation --------------------------//
ECS::ECS() {
	// Register all components here
	registerComponent<Transform>();
	registerComponent<ImageRenderer>();
	registerComponent<CircleRenderer>();
	registerComponent<RectRenderer>();
	registerComponent<AABBCollider>();
	registerComponent<AnimalBehavior>();
}

EntityID ECS::createEntity(std::string name, EntityID entityID) {
	EntityID newID;
	if (entityID != INVALID_ENTITY_ID) {
		auto it = std::find(deletedEntities.begin(), deletedEntities.end(), entityID);
		if (it != deletedEntities.end()) {
			newID = *it;
			deletedEntities.erase(it);
		}
		else {
			return INVALID_ENTITY_ID;
		}
	}
	else if (deletedEntities.size()) {
		EntityID deletedID = deletedEntities.front();
		deletedEntities.pop_front();
		newID = deletedID;
	}
	else {
		newID = totalEntityID;
		totalEntityID++;
	}

	if (name == "") name = "Entity " + std::to_string(newID);

	// Update entity metadata
	entityMetadataMap.insert({ newID, { {}, name } });
	allEntities.push_back(newID);
	return newID;
}

EntityID ECS::cloneEntity(EntityID entityID) {
	auto const& it = entityMetadataMap.find(entityID);
	assert(it != entityMetadataMap.end());

	EntityID newID = createEntity("copy");
	if (newID == INVALID_ENTITY_ID) {
		logger.error(std::string{ "ECS::cloneEntity - Failed to create new entity\nEntityID: " } + std::to_string(entityID) + "\n");
		return INVALID_ENTITY_ID;
	}

	for (auto const& componentID : it->second.componentIDs) {
		auto& pool = componentPools[componentID];
		if (!pool) {
			logger.error("ECS::cloneEntity - Invalid Component ID in entity metadata\n");
			return INVALID_ENTITY_ID;
		}

		if (pool->cloneComponent(entityID, newID)) {
			entityMetadataMap[newID].componentIDs.insert(componentID);
			auto componentData = getComponentData(componentID);

			if (componentData == componentMetadata.end()) {
				logger.error(std::string{ "ECS::cloneEntity - Component metadata did not contain data for component ID " } + std::to_string(componentID) + "\n");
				return INVALID_ENTITY_ID;
			}
			
			componentData->size += 1;
		}
	}
	reUpdateMetadata();
	return newID;
}

bool ECS::deleteEntity(EntityID entityID) {
	auto const& it = entityMetadataMap.find(entityID);
	if (it == entityMetadataMap.end()) return false;

	assert(it->first == entityID);

	for (auto const& componentID : it->second.componentIDs) {
		auto& pool = componentPools[componentID];
		if (!pool) continue;

		if (pool->removeComponent(entityID)) {
			auto componentDataIt = getComponentData(componentID);
			assert(componentDataIt != componentMetadata.end());
			componentDataIt->size -= 1;
		}
	}


	deletedEntities.push_back(entityID);
	entityMetadataMap.erase(entityMetadataMap.find(entityID));
	auto iterator = std::find_if(allEntities.begin(), allEntities.end(), [&](auto const& id) { return id == entityID; });
	assert(iterator != allEntities.end());

	allEntities.erase(iterator);
	reUpdateMetadata();
	return true;
}

std::string& ECS::getEntityName(EntityID entityID) {
	auto it = entityMetadataMap.find(entityID);
	assert(it != entityMetadataMap.end());
	return it->second.name;
}

EntityID ECS::findEntityByName(std::string const& name) const {
	for (auto const& [id, metadata] : entityMetadataMap) {
		if (metadata.name == name) return id;
	}
	return INVALID_ENTITY_ID;
}

std::vector<EntityID> const& ECS::getAllEntities() const {
	return allEntities;
}

bool ECS::removeComponent(EntityID entityID, ComponentID componentID) {
	auto it = getComponentData(componentID);
	assert(it != componentMetadata.end());
	return removeComponent(entityID, it->componentID);
}

std::vector<ComponentData>::const_iterator ECS::getComponentData(ComponentID componentID) const {
	return std::find_if(componentMetadata.begin(), componentMetadata.end(), [&](auto const& data) { return data.componentID == componentID; });
}

std::vector<ComponentData>::iterator ECS::getComponentData(ComponentID componentID) {
	return std::find_if(componentMetadata.begin(), componentMetadata.end(), [&](auto const& data) { return data.componentID == componentID; });
}

void ECS::reUpdateMetadata() {
	std::sort(componentMetadata.begin(), componentMetadata.end(), [](auto const& lhs, auto const& rhs) { return lhs.size < rhs.size; });
}