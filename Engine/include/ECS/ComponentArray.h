/*!------------------------------------------------------------------------------------------------
    File: ComponentArray.h
    Project: NocturnalStudio

	Brief:
		This file contains a templated class for storing an array of components based on the type. 
		This templated class is used for an ECS to map each entity to index and each index to entity 
		to ensure a packed array of components
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the component array class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "ECS/Types.h"
#include "pch.h"
#include "Components/Components.h"

namespace NocturnalEngine
{
	class IComponentArray
	{
	public:
		virtual ~IComponentArray() = default;
		virtual void EntityDestroyed(Entity entity) = 0;
		virtual void CopyComponent(Entity entity, Entity entityToCopy) = 0;
		virtual std::array<size_t, MAX_ENTITIES>& GetEntityToIndexMap() = 0;
		virtual std::array<Entity, MAX_ENTITIES>& GetIndexToEntityMap() = 0;
		virtual const size_t GetEntitySize() = 0;
		virtual void ClearForWorldBuild() = 0;
		virtual void Clear(const std::bitset<MAX_ENTITIES>& isPrefab) = 0;
	};

	template<typename T>
	class ComponentArray : public IComponentArray
	{
	public:
		inline ComponentArray()
		{
			std::fill(mEntityToIndexMap.begin(), mEntityToIndexMap.end(), MAX_ENTITIES);
			std::fill(mIndexToEntityMap.begin(), mIndexToEntityMap.end(), MAX_ENTITIES);
		}

		void InsertData(Entity entity, T component)
		{
			assert(mEntityToIndexMap[entity] == MAX_ENTITIES && "Component added to same entity more than once.");
			size_t newIndex = mSize;
			mEntityToIndexMap[entity] = newIndex;
			mIndexToEntityMap[newIndex] = entity;
			mComponentArray[newIndex] = component;
			component.SetComponentEntityID(entity);
			++mSize;
		}

		void RemoveData(Entity entity)
		{
			assert(mEntityToIndexMap[entity] != MAX_ENTITIES && "Removing non-existent component.");
			size_t indexOfRemovedEntity = mEntityToIndexMap[entity];
			size_t indexOfLastElement = mSize - 1;

			if (indexOfRemovedEntity != indexOfLastElement)
			{
				using std::swap;
				swap(mComponentArray[indexOfRemovedEntity], mComponentArray[indexOfLastElement]);
				Entity entityOfLastElement = mIndexToEntityMap[indexOfLastElement];
				mEntityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
				mIndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;
			}

			// Clear mappings for the removed entity and last slot
			mEntityToIndexMap[entity] = MAX_ENTITIES;
			mIndexToEntityMap[indexOfLastElement] = MAX_ENTITIES;

			// Explicitly destroy the now-unused last element and value-initialize it
			mComponentArray[indexOfLastElement].~T();
			new (&mComponentArray[indexOfLastElement]) T();

			--mSize;
		}

		T& GetData(Entity entity)
		{
			assert(mEntityToIndexMap[entity] != MAX_ENTITIES && "Retrieving non-existent component.");
			return mComponentArray[mEntityToIndexMap[entity]];
		}

		bool HaveComponent(Entity entity)
		{
			return mEntityToIndexMap[entity] != MAX_ENTITIES;
		}

		void EntityDestroyed(Entity entity) override
		{
			if (mEntityToIndexMap[entity] != MAX_ENTITIES)
			{
				RemoveData(entity);
			}
		}

		void CopyComponent(Entity entity, Entity entityToCopy) override
		{
			T* CopyComponent = &mComponentArray[mEntityToIndexMap[entity]];
			T* CloneComponent = new T(*CopyComponent);
			CloneComponent->SetComponentEntityID(entityToCopy);
			InsertData(entityToCopy, *CloneComponent);
			delete CloneComponent;
		}

		std::array<size_t, MAX_ENTITIES>& GetEntityToIndexMap()
		{
			return mEntityToIndexMap;
		}

		std::array<Entity, MAX_ENTITIES>& GetIndexToEntityMap()
		{
			return mIndexToEntityMap;
		}

		const size_t GetEntitySize()
		{
			return mSize;
		}

		void ClearForWorldBuild()
		{
			std::fill(mEntityToIndexMap.begin(), mEntityToIndexMap.end(), MAX_ENTITIES);
			std::fill(mIndexToEntityMap.begin(), mIndexToEntityMap.end(), MAX_ENTITIES);
		}

		void Clear(const std::bitset<MAX_ENTITIES>& isPrefab)
		{
			std::vector<Entity> entitiesToRemove;
			for (size_t i = 0; i < mSize; ++i)
			{
				Entity entity = mIndexToEntityMap[i];
				if (entity != MAX_ENTITIES && !isPrefab[entity])
				{
					entitiesToRemove.push_back(entity);
				}
			}
			for (auto it = entitiesToRemove.rbegin(); it != entitiesToRemove.rend(); ++it)
			{
				RemoveData(*it);
			}
		}

	private:
		std::array<T, MAX_ENTITIES> mComponentArray{};
		std::array<size_t, MAX_ENTITIES> mEntityToIndexMap;
		std::array<Entity, MAX_ENTITIES> mIndexToEntityMap;
		size_t mSize = 0;
	};
}
