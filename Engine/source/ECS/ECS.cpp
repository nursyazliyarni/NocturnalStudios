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
#include "Animation/SpriteAnimationComponent.h"
#include <fstream>
#include <sstream>

extern Logger logger;


//-------------------------- ECS Class Implementation --------------------------//

/**
 * @brief Construct an empty ECS manager and register built-in components.
 *
 * The constructor calls the internal registerComponent<T>() for the
 * commonly used engine component types so that the editor/runtime can
 * rely on those ComponentIDs being available.
 *
 * Example:
 *   // The global 'ecs' instance is constructed at program startup,
 *   // components are already registered when the instance is created.
 *   (void)ecs; // ensures linker references the global instance
 */
ECS::ECS() {
	// Register all components here
	registerComponent<Transform>();
	registerComponent<ImageRenderer>();
	registerComponent<SpriteRenderer>();
	registerComponent<SpriteAnimationComponent>();
	registerComponent<CircleRenderer>();
	registerComponent<RectRenderer>();
	registerComponent<AnimalBehavior>();
	registerComponent<AudioData>();
	registerComponent<PhysicsComponent>();
	registerComponent<AIComponent>();
	registerComponent<BoxCollider>();
	registerComponent<AABBCollider>();
	registerComponent<AIComponent>();

	ComponentFactory::Register<Transform>("Transform");
	ComponentFactory::Register<ImageRenderer>("ImageRenderer");
	ComponentFactory::Register<SpriteRenderer>("SpriteRenderer");
	ComponentFactory::Register<CircleRenderer>("CircleRenderer");
	ComponentFactory::Register<RectRenderer>("RectRenderer");
	ComponentFactory::Register<AnimalBehavior>("AnimalBehavior");
	ComponentFactory::Register<PhysicsComponent>("PhysicsComponent");
	ComponentFactory::Register<BoxCollider>("BoxCollider");
	ComponentFactory::Register<AABBCollider>("AABBCollider");
	ComponentFactory::Register<AIComponent>("AIComponent");

	// --------------------------------------------------------
	// Register JSON creation logic for SpriteAnimationComponent
	// --------------------------------------------------------
	ComponentFactory::Register("SpriteAnimationComponent", [](const rapidjson::Value& j) -> std::any {
		SpriteAnimationComponent comp;

		if (j.HasMember("anim_path")) {
			std::ifstream file(j["anim_path"].GetString());
			if (file) {
				std::stringstream buffer;
				buffer << file.rdbuf();
				rapidjson::Document animDoc;
				animDoc.Parse(buffer.str().c_str());

				for (auto& m : animDoc.GetObject()) {
                    SpriteAnimationComponent::Animation anim;
                    auto& obj = m.value;

                    if (obj.HasMember("frameCount"))
                        anim.frames = obj["frameCount"].GetInt();
                    if (obj.HasMember("frameTime"))
                        anim.frameTime = obj["frameTime"].GetFloat();
                    if (obj.HasMember("loop"))
                        anim.loop = obj["loop"].GetBool();

                    // NEW: per-animation data
                    if (obj.HasMember("sprite_path") && obj["sprite_path"].IsString())
                        anim.spritePath = obj["sprite_path"].GetString();
                    if (obj.HasMember("sheetColumns") && obj["sheetColumns"].IsInt())
                        anim.sheetColumns = obj["sheetColumns"].GetInt();
                    if (obj.HasMember("sheetRows") && obj["sheetRows"].IsInt())
                        anim.sheetRows = obj["sheetRows"].GetInt();

                    comp.animations[m.name.GetString()] = anim;
                }
			}
		}

		if (j.HasMember("currentAnim")) comp.currentAnim = j["currentAnim"].GetString();
		if (j.HasMember("sheetColumns")) comp.sheetColumns = j["sheetColumns"].GetInt();
		if (j.HasMember("sheetRows")) comp.sheetRows = j["sheetRows"].GetInt();

		comp.playing = true;
		comp.timer = 0.0f;
		comp.currentFrame = 0;
		return comp;
		});
}

/**
 * @brief Create a new entity and register its metadata.
 *
 * If an explicit non-INVALID_ENTITY_ID is provided, the function will
 * attempt to reuse that ID only if it previously existed and was deleted.
 * Otherwise a recycled ID from the deletedEntities list is used (if any),
 * or a new ID is allocated from totalEntityID.
 *
 * The function inserts a default EntityMetadata entry with the provided
 * name (or "Entity <id>" when name is empty) and appends the ID to
 * the allEntities list.
 *
 * @param name Human readable name for the entity. If empty, a default
 *             name "Entity <id>" is assigned.
 * @param entityID Optional explicit EntityID to try to use. Pass
 *                 INVALID_ENTITY_ID to auto-allocate.
 * @return The created EntityID or INVALID_ENTITY_ID on failure.
 *
 * Simple usage:
 *   EntityID player = ecs.createEntity("Player");
 *   if (player == INVALID_ENTITY_ID) { /* handle error *\/ }
 *
 * Using an explicit (previously deleted) id:
 *   EntityID reused = ecs.createEntity("ReusedEntity", previouslyDeletedId);
 */
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

/**
 * @brief Clone an existing entity and copy its components.
 *
 * The clone operation:
 *  - Creates a new entity named "copy".
 *  - Iterates the source entity's component IDs and asks each component
 *    pool to clone the component instance from the source entity to the
 *    newly created entity.
 *  - Updates component count metadata and the new entity's metadata set.
 *
 * The function returns INVALID_ENTITY_ID on any failure (failed creation,
 * missing pool, or missing metadata).
 *
 * @param entityID Source entity to clone.
 * @return New entity ID on success, or INVALID_ENTITY_ID on failure.
 *
 * Simple usage:
 *   EntityID enemy = ecs.createEntity("Enemy");
 *   // ... add components to 'enemy' ...
 *   EntityID enemyCopy = ecs.cloneEntity(enemy);
 *   if (enemyCopy == INVALID_ENTITY_ID) { /* clone failed *\/ }
 */
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

/**
 * @brief Delete an entity and remove all its components.
 *
 * The function:
 *  - Validates the entity exists.
 *  - Iterates the entity's attached components and asks each component
 *    pool to remove the component instance.
 *  - Decrements the component type's stored size in metadata for each
 *    successfully removed component.
 *  - Removes the entity's metadata and adds the entity ID to the
 *    deletedEntities list for later reuse.
 *  - Updates cached metadata ordering by calling reUpdateMetadata().
 *
 * @param entityID The entity to delete.
 * @return true on success; false if the entity did not exist.
 *
 * Simple usage:
 *   if (!ecs.deleteEntity(playerId)) {
 *     // entity did not exist or deletion failed
 *   }
 */
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

/**
 * @brief Return a mutable reference to an entity's stored name.
 *
 * The caller may modify the returned string to rename the entity.
 * This function asserts that the entity exists.
 *
 * @param entityID The entity whose name to retrieve.
 * @return Reference to the entity's name string.
 *
 * Simple usage:
 *   EntityID id = ecs.createEntity("OldName");
 *   std::string& n = ecs.getEntityName(id);
 *   n = "NewName"; // rename in place
 */
std::string& ECS::getEntityName(EntityID entityID) {
	auto it = entityMetadataMap.find(entityID);
	assert(it != entityMetadataMap.end());
	return it->second.name;
}


/**
 * @brief Find an entity by its stored name.
 *
 * Performs a linear search over entity metadata and returns the first
 * entity with a matching name. If not found, returns INVALID_ENTITY_ID.
 *
 * @param name Name to search for.
 * @return Matching EntityID or INVALID_ENTITY_ID if none found.
 *
 * Simple usage:
 *   EntityID id = ecs.findEntityByName("Player");
 *   if (id != INVALID_ENTITY_ID) { /* found *\/ }
 */
EntityID ECS::findEntityByName(std::string const& name) const {
	for (auto const& [id, metadata] : entityMetadataMap) {
		if (metadata.name == name) return id;
	}
	return INVALID_ENTITY_ID;
}

/**
 * @brief Get a const reference to the list of all active entities.
 *
 * The order reflects insertion and potential recycled IDs; callers should
 * not depend on any particular ordering beyond what the implementation
 * currently provides.
 *
 * @return const reference to the vector of all entity IDs.
 *
 * Simple usage:
 *   for (auto id : ecs.getAllEntities()) { /* iterate entities *\/ }
 */
std::vector<EntityID> const& ECS::getAllEntities() const {
	return allEntities;
}

/**
 * @brief Remove a component from an entity using a ComponentID.
 *
 * Non-templated variant intended for runtime/editor systems that only
 * know a component's numeric ID. This function locates the ComponentData
 * entry and then forwards to the templated/non-templated remove routine.
 *
 * @param entityID Target entity.
 * @param componentID Component type ID to remove.
 * @return true on success; false on failure.
 *
 * Simple usage:
 *   ComponentID cid = Family::getID<Health>();
 *   ecs.removeComponent(playerId, cid);
 */
bool ECS::removeComponent(EntityID entityID, ComponentID componentID) {
	auto it = getComponentData(componentID);
	assert(it != componentMetadata.end());

	// Find the component pool for the given componentID
	auto poolIt = componentPools.find(componentID);
	if (poolIt == componentPools.end() || !poolIt->second) {
		logger.error("ECS::removeComponent - Component pool not found for given componentID\n");
		return false;
	}

	// Remove the component from the pool
	if (!poolIt->second->removeComponent(entityID)) {
		logger.error(std::string{ "ECS::removeComponent - Failed to remove component from entity\nEntityID: " } + std::to_string(entityID) + "\n");
		return false;
	}

	// Update metadata
	it->size -= 1;
	auto metaIt = entityMetadataMap.find(entityID);
	if (metaIt != entityMetadataMap.end()) {
		metaIt->second.componentIDs.erase(componentID);
	}
	reUpdateMetadata();
	return true;
}

/**
 * @brief Find the ComponentData entry for a component ID (const).
 *
 * Returns an iterator to the matching entry in componentMetadata or
 * componentMetadata.end() if not found.
 *
 * @param componentID Component type identifier.
 * @return const_iterator to the found ComponentData or componentMetadata.end().
 *
 * Simple usage:
 *   auto it = ecs.getComponentData(Family::getID<Transform>();
 *   if (it != ecs.componentMetadata.end()) { size_t count = it->size; }
 */
std::vector<ComponentData>::const_iterator ECS::getComponentData(ComponentID componentID) const {
	return std::find_if(componentMetadata.begin(), componentMetadata.end(), [&](auto const& data) { return data.componentID == componentID; });
}

/**
 * @brief Find the ComponentData entry for a component ID (mutable).
 *
 * Same as the const variant but returns a mutable iterator for operations
 * that modify the ComponentData (e.g. incrementing/decrementing size).
 *
 * @param componentID Component type identifier.
 * @return iterator to the found ComponentData or componentMetadata.end().
 *
 * Simple usage:
 *   auto it = ecs.getComponentData(Family::getID<Transform>();
 *   if (it != ecs.componentMetadata.end()) { it->size += 1; }
 */
std::vector<ComponentData>::iterator ECS::getComponentData(ComponentID componentID) {
	return std::find_if(componentMetadata.begin(), componentMetadata.end(), [&](auto const& data) { return data.componentID == componentID; });
}

/**
 * @brief Recompute cached metadata ordering.
 *
 * The current implementation sorts the vector of ComponentData by the
 * number of stored instances (`size`). This is an internal helper called
 * after operations that change component counts. It is an implementation
 * detail and not intended for public use.
 *
 * Simple usage:
 *   // Typically called internally; manual invocation is rarely needed:
 *   ecs.reUpdateMetadata();
 */
void ECS::reUpdateMetadata() {
	std::sort(componentMetadata.begin(), componentMetadata.end(), [](auto const& lhs, auto const& rhs) { return lhs.size < rhs.size; });
}
