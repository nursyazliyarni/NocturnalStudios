#ifndef ECS_COMPONENTPOOL_H
#define ECS_COMPONENTPOOL_H

/*!------------------------------------------------------------------------------------------------
	File: ECS.h
	Project: NocturnalStudio

	This file contains the defination of template class ComponentPool, which represents all the
	component data of a specific component type. It provides APIs to add and remove component data
	while keeping track of which entity does the component data belong to.

	Primary Author:
		
		

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

class BaseComponentPool {
public:
	BaseComponentPool() = default;

	virtual ~BaseComponentPool() = 0 {}

	BaseComponentPool(BaseComponentPool const&) = delete;
	BaseComponentPool(BaseComponentPool&&) = delete;
	BaseComponentPool& operator=(BaseComponentPool const&) = delete;
	BaseComponentPool& operator=(BaseComponentPool&&) = delete;

	std::vector<EntityID> const& getAllEntities() {
		return { denseEntities };
	}
	virtual bool removeComponent(EntityID ID) = 0;
	virtual bool cloneComponent(EntityID srcID, EntityID newID) = 0;
	virtual std::unique_ptr<BaseComponentPool> clone() const = 0;

protected:
	std::vector<EntityID> denseEntities;
	std::vector<EntityID> sparseEntities;
};

template <typename T>
class ComponentPool : public BaseComponentPool {
public:
	ComponentPool() : BaseComponentPool() {};

	std::vector<T> const& getAllComponents() const {
		return components;
	}

	bool hasComponent(EntityID ID) const {
		if (ID >= sparseEntities.size()) return false;
		if (sparseEntities[ID] >= denseEntities.size()) return false;

		return denseEntities[sparseEntities[ID]] == ID;
	}

	T* getComponent(EntityID ID) {
		if (ID >= sparseEntities.size()) return nullptr;

		std::size_t index = sparseEntities[ID];
		if (index >= denseEntities.size() || denseEntities[index] != ID) return nullptr;
		
		return &components[index];
	}

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

	bool cloneComponent(EntityID srcID, EntityID newID) final {
		if (srcID >= sparseEntities.size()) return false;
		return addComponent(newID, components[sparseEntities[srcID]]);
	}

	std::unique_ptr<BaseComponentPool> clone() const final {
		std::unique_ptr<ComponentPool<T>> newPool = std::make_unique<ComponentPool<T>>();
		std::copy(components.begin(), components.end(), std::back_inserter(newPool->components));
		std::copy(denseEntities.begin(), denseEntities.end(), std::back_inserter(newPool->denseEntities));
		std::copy(sparseEntities.begin(), sparseEntities.end(), std::back_inserter(newPool->sparseEntities));

		return std::move(newPool);
	}

private:
	std::vector<T> components;
};

#endif // COMPONENTPOOL_H