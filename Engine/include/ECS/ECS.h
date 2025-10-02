#ifndef ECS_H
#define ECS_H

/*!------------------------------------------------------------------------------------------------
	File: ECS.h
	Project: NocturnalStudio

	This file contains the class definition for the Entity Component System (ECS). ECS is a game
	object manager that employs the Entity-Component-System design pattern.

	Primary Author:
		
		

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/


#include <vector>
#include <memory>
#include <list>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <optional>
#include <any>
#include <functional>

#include "../include/ECS/ECS_Component.h"
#include "../include/ECS/ECS_ComponentPool.h"
#include "../include/ECS/ECS_Def.h"

class ECS {
public:
	ECS();

	EntityID createEntity(std::string name = "", EntityID entityID = INVALID_ENTITY_ID);
	EntityID cloneEntity(EntityID entityID);
	bool deleteEntity(EntityID entityID);

	std::string& getEntityName(EntityID entityID);
	EntityID findEntityByName(const std::string& name) const;
	std::vector<EntityID> const& getAllEntities() const;

	template <typename T>
	bool addComponent(EntityID entityID, T const& component);
	template <typename T>
	void registerComponent();
	template <typename T>
	bool hasComponent(EntityID entityID) const;
	template <typename T>
	bool removeComponent(EntityID entityID);
	bool removeComponent(EntityID entityID, ComponentID componentID);
	template <typename T>
	T* getComponent(EntityID entityID);

private:
	std::vector<ComponentData>::const_iterator getComponentData(ComponentID componentID) const;
	std::vector<ComponentData>::iterator getComponentData(ComponentID componentID);
	void reUpdateMetadata();

	EntityID totalEntityID = 0;
	std::list<EntityID> deletedEntities;
	std::vector<EntityID> allEntities;
	std::unordered_map<EntityID, EntityMetadata> entityMetadataMap;

	std::unordered_map<ComponentID, std::unique_ptr<BaseComponentPool>> componentPools;
	std::vector<ComponentData> componentMetadata;
	std::unordered_map<std::string, ComponentID> componentNameToIDMap;

};

//********* ECS Class Template Implementation **********//

template <typename T>
bool ECS::addComponent(EntityID entityID, T const& component) {
	ComponentID type = Family::getID<T>();
	if (componentPools.find(type) == componentPools.end()) registerComponent<T>();

	BaseComponentPool* pool = componentPools.at(type).get();
	ComponentPool<T>* specificPool = dynamic_cast<ComponentPool<T>*>(pool);
	if (!specificPool) {
		logger.error(std::string{ "ECS::addComponent - Failed to derive correct component pool\nComponentID: " } + std::to_string(type) + "\n");
		return false;
	}
	if (!specificPool->addComponent(entityID, component)) {
		logger.error(std::string{ "ECS::addComponent - Failed to add component to entity\nEntityID: " } + std::to_string(entityID) + "\n");
		return false;
	}

	auto componentData = getComponentData(type);
	if (componentData == componentMetadata.end()) {
		logger.error(std::string{ "ECS::addComponent - Failed to find component metadata\nComponentID: " } + std::to_string(type) + "\n");
		return false;
	}

	componentData->size += 1;
	entityMetadataMap[entityID].componentIDs.insert(type);
	reUpdateMetadata();
	return true;
}

template <typename T>
void ECS::registerComponent() {
	ComponentID type = Family::getID<T>();
	auto [it, inserted] = componentPools.insert({ type, std::make_unique<ComponentPool<T>>() });
	if (!inserted) {
		logger.warning(std::string{ "ECS::registerComponent - Component already registered" });
		return;
	}

	componentMetadata.push_back({ type, 0 });

	// TODO: May require a static name member in every component for readable names
	componentNameToIDMap.insert({ typeid(T).name(), type });
}

template <typename T>
bool ECS::removeComponent(EntityID entityID) {
	ComponentID type = Family::getID<T>();
	BaseComponentPool* pool;
	if (componentPools.find(type) == componentPools.end()) {
		logger.error(std::string{ "ECS::removeComponent - Failed to find component pool" });
		return false;
	}

	pool = componentPools[type].get();
	ComponentPool<T>* specificPool = dynamic_cast<ComponentPool<T>*>(pool);
	if (!specificPool) {
		logger.error(std::string{ "ECS::removeComponent - Failed to derive correct component pool\nComponentID: " } + std::to_string(type) + "\n");
		return false;
	}
	if (!specificPool->removeComponent(entityID)) {
		logger.error(std::string{ "ECS::removeComponent - Failed to remove component from entity\nEntityID: " } + std::to_string(entityID) + "\n");
		return false;
	}

	auto componentData = getComponentData(type);
	if (componentData == componentMetadata.end()) {
		logger.error(std::string{ "ECS::removeComponent - Failed to find component metadata\nComponentID: " } + std::to_string(type) + "\n");
		return false;
	}

	componentData->size -= 1;
	entityMetadataMap[entityID].componentIDs.erase(type);
	reUpdateMetadata();
	return true;
}

template <typename T>
T* ECS::getComponent(EntityID entityID) {
	ComponentID type = Family::getID<T>();
	if (componentPools.find(type) == componentPools.end()) {
		logger.error(std::string{ "ECS::getComponent - Failed to find component pool\nComponentID: " } + std::to_string(type) + "\n");
		return nullptr;
	}
	BaseComponentPool* pool = componentPools[type].get();
	ComponentPool<T>* specificPool = dynamic_cast<ComponentPool<T>*>(pool);
	if (!specificPool) {
		logger.error(std::string{ "ECS::getComponent - Failed to derive correct component pool\nComponentID: " } + std::to_string(type) + "\n");
		return nullptr;
	}
	return specificPool->getComponent(entityID);
}

#endif // ECS_H