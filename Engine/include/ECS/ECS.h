#ifndef ECS_H
#define ECS_H

/*!------------------------------------------------------------------------------------------------
	File: ECS.h
	Project: NocturnalStudio

	This file contains the class definition for the Entity Component System (ECS). ECS is a game
	object manager that employs the Entity-Component-System design pattern.

	ECS also manages scenes, layers and prefabs (if needed), providing users the interface to edit
	the game objects in the scene.

	Primary Author:
		Lau Xiang Yang (100%) (141 lns as of 20/10/2025)
		- Created the file and implemented the ECS class member functions


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

#include <rapidjson/document.h>
#include "../include/ECS/ECS_Component.h"
#include "../include/ECS/ECS_ComponentPool.h"
#include "../include/ECS/ECS_Def.h"



class ECS {
public:
	/**
	 * singleton ECS caller.
	 *
	 * \return
	 */
	static ECS& getECS() {
		static ECS dis;
		return dis;
	}

	/**
	 * @brief Create a new entity.
	 *
	 * Creates a new entity and registers it in the ECS. If a non-zero valid
	 * EntityID is provided it will attempt to use that ID (useful when loading
	 * or restoring saved scenes). The provided name is stored in the entity's metadata.
	 *
	 * @param name Optional human-readable name for the entity.
	 * @param entityID Optional explicit EntityID to assign. Pass INVALID_ENTITY_ID
	 *                 (default) to auto-allocate a new ID.
	 * @return The created entity's EntityID, or INVALID_ENTITY_ID on failure.
	 *
	 * Simple usage:
	 *   EntityID player = ecs.createEntity("Player");
	 *   EntityID copy  = ecs.createEntity("ClonedPlayer", suggestedId);
	 */
	EntityID createEntity(std::string name = "", EntityID entityID = INVALID_ENTITY_ID);

	/**
	 * @brief Clone an existing entity.
	 *
	 * Creates a new entity and copies all components from the source entity.
	 * Component copying is delegated to the component pools' clone routines.
	 *
	 * @param entityID The source entity to clone.
	 * @return The new entity's ID, or INVALID_ENTITY_ID on failure.
	 *
	 * Simple usage:
	 *   EntityID enemy = ecs.createEntity("Enemy");
	 *   // ... add components to 'enemy'
	 *   EntityID enemyCopy = ecs.cloneEntity(enemy);
	 */
	EntityID cloneEntity(EntityID entityID);

	/**
	 * @brief Delete an entity and remove its components.
	 *
	 * Marks the entity for deletion, removes components and frees its metadata.
	 * Returns true on success.
	 *
	 * @param entityID The entity to delete.
	 * @return true if the entity was successfully deleted; false otherwise.
	 *
	 * Simple usage:
	 *   if (!ecs.deleteEntity(playerId)) { /* handle error *\/ }
	 */
	bool deleteEntity(EntityID entityID);

	/**
	 * @brief Get the name assigned to an entity.
	 *
	 * Returns a reference to the stored name. Behavior is undefined if the
	 * entity does not exist — check existence before calling if uncertain.
	 *
	 * @param entityID The entity to query.
	 * @return Reference to the entity name string.
	 *
	 * Simple usage:
	 *   std::string& name = ecs.getEntityName(playerId);
	 *   std::cout << "Name: " << name << '\n';
	 */
	std::string& getEntityName(EntityID entityID);

	/**
	 * @brief Find an entity by its stored name.
	 *
	 * Returns the EntityID for the first matching entity name or INVALID_ENTITY_ID
	 * if no entity matches.
	 *
	 * @param name The name to search for.
	 * @return Matching EntityID or INVALID_ENTITY_ID.
	 *
	 * Simple usage:
	 *   EntityID id = ecs.findEntityByName("Player");
	 *   if (id != INVALID_ENTITY_ID) { /* found *\/ }
	 */
	EntityID findEntityByName(const std::string& name) const;

	/**
	 * @brief Retrieve the list of all active entity IDs.
	 *
	 * Returns a const reference to the internal vector of entity IDs. Do not modify
	 * the returned vector directly.
	 *
	 * @return const reference to vector of EntityID
	 *
	 * Simple usage:
	 *   for (EntityID id : ecs.getAllEntities()) { /* iterate entities *\/ }
	 */
	std::vector<EntityID> const& getAllEntities() const;

	/**
	 * @brief Attach a component of type T to an entity.
	 *
	 * If the component pool for T is not registered it will be registered
	 * automatically. Returns true on success.
	 *
	 * @tparam T Component type (a POD/struct representing component data)
	 * @param entityID Target entity
	 * @param component The component instance to copy into the pool
	 * @return true on success; false on failure
	 *
	 * Simple usage:
	 *   Transform t{ ... };
	 *   ecs.addComponent<Transform>(playerId, t);
	 */
	template <typename T>
	bool addComponent(EntityID entityID, T const& component);

	/**
	 * @brief Register a component type T with the ECS.
	 *
	 * Creates an internal ComponentPool for T and registers metadata. This
	 * is done automatically by addComponent if needed, but you can call it
	 * early to ensure pools exist before loading data.
	 *
	 * @tparam T Component type
	 *
	 * Simple usage:
	 *   ecs.registerComponent<Transform>();
	 */
	template <typename T>
	void registerComponent();

	/**
	 * @brief Query whether an entity has a component of type T.
	 *
	 * @tparam T Component type
	 * @param entityID Entity to query
	 * @return true if entity has component T; false otherwise
	 *
	 * Simple usage:
	 *   if (ecs.hasComponent<Health>(enemyId)) { /* has health *\/ }
	 */
	template <typename T>
	bool hasComponent(EntityID entityID) const;

	/**
	 * @brief Remove component of type T from an entity.
	 *
	 * @tparam T Component type
	 * @param entityID Target entity
	 * @return true on success; false on failure
	 *
	 * Simple usage:
	 *   ecs.removeComponent<Weapon>(playerId);
	 */
	template <typename T>
	bool removeComponent(EntityID entityID);

	/**
	 * @brief Remove a component from an entity by component ID.
	 *
	 * Non-templated API variant that uses a ComponentID, useful for editors
	 * or serialized systems where only numeric IDs are known at runtime.
	 *
	 * @param entityID Target entity
	 * @param componentID ComponentID (use Family::getID<T>() to obtain)
	 * @return true on success; false on failure
	 *
	 * Simple usage:
	 *   ComponentID cid = Family::getID<Health>();
	 *   ecs.removeComponent(playerId, cid);
	 */
	bool removeComponent(EntityID entityID, ComponentID componentID);

	/**
	 * @brief Get pointer to component T for an entity.
	 *
	 * Returns a pointer to the component stored in the pool or nullptr if the
	 * entity does not have that component or the pool is not registered.
	 *
	 * @tparam T Component type
	 * @param entityID Target entity
	 * @return Pointer to component instance or nullptr
	 *
	 * Simple usage:
	 *   if (auto* h = ecs.getComponent<Health>(playerId)) {
	 *       h->hp -= 10;
	 *   }
	 */
	template <typename T>
	T* getComponent(EntityID entityID);

	bool emptyAllEntities();

private:
	/**
	 * @brief Find the ComponentData entry for a component ID (const).
	 *
	 * Returns an iterator to the matching entry in componentMetadata or
	 * componentMetadata.end() if not found.
	 */
	std::vector<ComponentData>::const_iterator getComponentData(ComponentID componentID) const;
	std::vector<ComponentData>::iterator getComponentData(ComponentID componentID);

	/**
	 * @brief Recompute any cached metadata that depends on components/entities.
	 *
	 * This function is called internally when components are added/removed.
	 * It should be considered implementation detail and is not intended for
	 * public use.
	 */
	void reUpdateMetadata();


	ECS(); //singleton, constructor is private.

	ECS(const ECS&) = delete;
	ECS& operator=(const ECS&) = delete;

	ECS(ECS&&) = delete;
	ECS& operator=(ECS&&) = delete;

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
	assert(specificPool != nullptr);

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




// [====================*====================]
//  Component Factory for Json Parsing
// [====================*====================]

//check compatability with the register component function
//if this section causes any problems, comment it out
class ComponentFactory { //basically copied my component factory but modified it for structs, I need to check how to do the loading afterwards
public:
	using Json = rapidjson::Value;
	using Creator = std::function<std::any(const Json&)>;

	static void Register(const std::string& name, Creator creator) {
		GetMap()[name] = std::move(creator);
	}

	static std::any Create(const std::string& name, const Json& j) {
		auto& map = GetMap();
		auto it = map.find(name);
		if (it != map.end()) return it->second(j);
		throw std::runtime_error("Unknown component type: " + name);
	}

private:
	static std::unordered_map<std::string, Creator>& GetMap() {
		static std::unordered_map<std::string, Creator> map;
		return map;
	}
};
#endif // ECS_H