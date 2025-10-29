#ifndef ECS_COMPONENTPOOL_H
#define ECS_COMPONENTPOOL_H

/*!------------------------------------------------------------------------------------------------
	File: ECS_ComponentPool.h
	Project: NocturnalStudio

	This file contains the defination of template class ComponentPool, which represents all the
	component data of a specific component type. It provides APIs to add and remove component data
	while keeping track of which entity does the component data belong to.

	Primary Author:
		Lau Xiang Yang (100%)
		- Created the file and implemented the ComponentPool class


	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <unordered_map>

#include "ECS_Def.h"
#include "../include/core/Logger.h"

extern Logger logger;

/**
 * @brief Non-templated base for component pools.
 *
 * `BaseComponentPool` exposes a small, type-erased API used by the ECS to
 * manage per-component-type storage without knowing the concrete component
 * type at compile time.
 *
 * Typical usage (internal to ECS):
 *   // The ECS stores pointers to BaseComponentPool. Systems should not use
 *   // BaseComponentPool directly for typed access; prefer ComponentPool<T>.
 */
class BaseComponentPool {
public:
	BaseComponentPool() = default;

	/// Virtual destructor required for polymorphic deletion.
	virtual ~BaseComponentPool() = 0 {}

	BaseComponentPool(BaseComponentPool const&) = delete;
	BaseComponentPool(BaseComponentPool&&) = delete;
	BaseComponentPool& operator=(BaseComponentPool const&) = delete;
	BaseComponentPool& operator=(BaseComponentPool&&) = delete;

	/**
	 * @brief Get all entity IDs that have a component in this pool.
	 *
	 * Returns a const reference to the internal dense list of entity IDs.
	 * The returned reference is valid as long as the pool object exists and
	 * is not modified (adding/removing components may reallocate the vector).
	 *
	 * Usage:
	 *   auto const& entities = pool->getAllEntities();
	 *   for (EntityID id : entities) { /* iterate components *\/ }
	 */
	std::vector<EntityID> const& getAllEntities() {
		return denseEntities;
	}

	/**
	 * @brief Remove the component for a given entity ID.
	 *
	 * Implementations should remove component data for `ID` if present and
	 * update internal dense/sparse structures accordingly.
	 *
	 * @param ID EntityID to remove the component from.
	 * @return true when a component was removed; false if none existed.
	 *
	 * Usage (internal):
	 *   bool removed = pool->removeComponent(myEntity);
	 */
	virtual bool removeComponent(EntityID ID) = 0;

	/**
	 * @brief Clone a component from `srcID` into `newID`.
	 *
	 * Implementations should duplicate component data associated with `srcID`
	 * and attach it to `newID`. Used by entity cloning logic.
	 *
	 * @param srcID Source entity that currently owns the component.
	 * @param newID Destination entity to receive the cloned component.
	 * @return true on success; false otherwise.
	 *
	 * Usage (internal):
	 *   pool->cloneComponent(sourceEntity, clonedEntity);
	 */
	virtual bool cloneComponent(EntityID srcID, EntityID newID) = 0;

	/**
	 * @brief Produce a deep copy of the pool (type-erased).
	 *
	 * Used when the ECS needs to duplicate component pools (for snapshots,
	 * scene copies, or similar operations). The returned unique_ptr holds a
	 * new pool instance containing copies of all stored components and
	 * dense/sparse structures.
	 *
	 * @return unique_ptr to a cloned BaseComponentPool
	 */
	virtual std::unique_ptr<BaseComponentPool> clone() const = 0;

protected:
	/// Dense array of entity IDs that currently have a component in this pool.
	std::vector<EntityID> denseEntities;

	/// Sparse map indexed by EntityID -> index into denseEntities/components
	std::vector<EntityID> sparseEntities;
};

/**
 * @brief Typed component storage following a sparse-set pattern.
 *
 * `ComponentPool<T>` stores components of type `T` together with a sparse/dense
 * mapping to provide O(1) add/remove/lookup by EntityID while keeping
 * component data tightly packed for cache-friendly iteration.
 *
 * Example usage:
 *   ComponentPool<Transform> pool;
 *   Transform t{...};
 *   pool.addComponent(entityId, t);
 *   if (pool.hasComponent(entityId)) {
 *       auto* ptr = pool.getComponent(entityId);
 *       ptr->position.x += 1.0f;
 *   }
 */
template <typename T>
class ComponentPool : public BaseComponentPool {
public:
	ComponentPool() : BaseComponentPool() {};

	/**
	 * @brief Get a const reference to all stored components.
	 *
	 * Components are stored in the same order as `denseEntities`. The i-th
	 * component corresponds to the entity ID stored at `denseEntities[i]`.
	 *
	 * Usage:
	 *   auto const& comps = pool.getAllComponents();
	 *   for (const auto& c : comps) { /* iterate *\/ }
	 */
	std::vector<T> const& getAllComponents() const {
		return components;
	}

	/**
	 * @brief Query whether a given entity has a component of type T.
	 *
	 * Performs bounds checks on the sparse array and verifies the dense mapping.
	 *
	 * @param ID EntityID to query.
	 * @return true if the entity has a component in this pool; false otherwise.
	 *
	 * Usage:
	 *   if (pool.hasComponent(entity)) { /* safe to call getComponent *\/ }
	 */
	bool hasComponent(EntityID ID) const {
		if (ID >= sparseEntities.size()) return false;
		if (sparseEntities[ID] >= denseEntities.size()) return false;

		return denseEntities[sparseEntities[ID]] == ID;
	}

	/**
	 * @brief Get pointer to the component instance for an entity.
	 *
	 * Returns nullptr if the entity does not have a component of this type.
	 *
	 * @param ID EntityID to query.
	 * @return pointer to component T or nullptr.
	 *
	 * Usage:
	 *   T* comp = pool.getComponent(myEntity);
	 *   if (comp) comp->...;
	 */
	T* getComponent(EntityID ID) {
		if (ID >= sparseEntities.size()) return nullptr;

		std::size_t index = sparseEntities[ID];
		if (index >= denseEntities.size() || denseEntities[index] != ID) return nullptr;

		return &components[index];
	}

	/**
	 * @brief Add a component instance for an entity.
	 *
	 * Appends the component to the dense array and updates the sparse mapping.
	 * Adding a component that already exists for `ID` fails and returns false.
	 *
	 * @param ID EntityID to attach the component to (must not be INVALID_ENTITY_ID).
	 * @param component Instance (copied) to store.
	 * @return true on success; false if the entity already has the component or on invalid ID.
	 *
	 * Usage:
	 *   Transform t{...};
	 *   bool ok = pool.addComponent(entity, t);
	 */
	bool addComponent(EntityID ID, T const& component) {
		if (ID == INVALID_ENTITY_ID) {
			logger.error("ComponentPool::addComponent - Attempting to add component to invalid entity ID");
			return false;
		}

		std::size_t index = denseEntities.size();
		if (sparseEntities.size() < ID + 1) {
			sparseEntities.resize(ID + 1, INVALID_ENTITY_ID);
		}

		if (sparseEntities[ID] < denseEntities.size() && denseEntities[sparseEntities[ID]] == ID) {
			logger.error("ComponentPool::addComponent - Entity already has component");
			return false;
		}

		sparseEntities[ID] = index;
		denseEntities.push_back(ID);
		components.push_back(component);
		return true;
	}

	/**
	 * @brief Remove the component for an entity.
	 *
	 * Removal uses the common swap-with-last strategy to maintain dense packing.
	 * Returns false when the entity had no component.
	 *
	 * @param ID EntityID to remove the component from.
	 * @return true if the component was removed; false otherwise.
	 *
	 * Usage:
	 *   if (!pool.removeComponent(entity)) { /* maybe already removed *\/ }
	 */
	bool removeComponent(EntityID ID) final {
		if (ID >= sparseEntities.size()) return false;
		if (!denseEntities.size()) return false;

		EntityID lastEntity = denseEntities.back();
		EntityID index = sparseEntities[ID];
		if (index >= denseEntities.size()) return false;
		if (index == INVALID_ENTITY_ID || ID != denseEntities[index]) return false;

		// Remove the component by swapping with the last element
		std::swap(components.back(), components[sparseEntities[ID]]);
		components.pop_back();

		std::swap(denseEntities.back(), denseEntities[sparseEntities[ID]]);
		denseEntities.pop_back();

		std::swap(sparseEntities[lastEntity], sparseEntities[ID]);

		return true;
	}

	/**
	 * @brief Clone a component from `srcID` into `newID`.
	 *
	 * Helper used when cloning an entity; this reuses `addComponent` to copy
	 * the concrete component value stored for `srcID`.
	 *
	 * @param srcID Existing entity holding the component to clone.
	 * @param newID Destination entity to receive the copied component.
	 * @return true on success; false otherwise.
	 *
	 * Usage (internal):
	 *   pool.cloneComponent(existingEntity, newEntity);
	 */
	bool cloneComponent(EntityID srcID, EntityID newID) final {
		if (srcID >= sparseEntities.size()) return false;
		return addComponent(newID, components[sparseEntities[srcID]]);
	}

	/**
	 * @brief Create a deep copy of this typed pool.
	 *
	 * Copies components, denseEntities and sparseEntities into a new pool
	 * instance and returns it as a `unique_ptr<BaseComponentPool>`.
	 *
	 * @return unique_ptr owning the cloned typed pool.
	 *
	 * Usage (internal):
	 *   auto clonedPool = pool.clone();
	 */
	std::unique_ptr<BaseComponentPool> clone() const final {
		std::unique_ptr<ComponentPool<T>> newPool = std::make_unique<ComponentPool<T>>();
		std::copy(components.begin(), components.end(), std::back_inserter(newPool->components));
		std::copy(denseEntities.begin(), denseEntities.end(), std::back_inserter(newPool->denseEntities));
		std::copy(sparseEntities.begin(), sparseEntities.end(), std::back_inserter(newPool->sparseEntities));

		return std::move(newPool);
	}

private:
	/// Packed component storage: components[i] corresponds to denseEntities[i]
	std::vector<T> components;
};

#endif // COMPONENTPOOL_H