/*!------------------------------------------------------------------------------------------------
    File: ECS.h
    Project: NocturnalStudio

	Brief:
		This file contains the coordinator for the three systems in place
		that manages the ECS, the component, entity and system manager communicates
		with each other using this class
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the ECS class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "ComponentManager.h"
#include "EntityManager.h"
#include "System/SystemManager.h"
#include "Types.h"
#include "pch.h"
#include "Core/MemoryManager.h"

#define p_ecs NocturnalEngine::ECS::GetInstance()

namespace NocturnalEngine
{
	class ECS
	{
	public:

		/*!*************************************************************************
		Returns an instance of the ECS
		****************************************************************************/
		static ECS& GetInstance()
		{
			static ECS instance;
			return instance;
		}

		/*!*************************************************************************
		Initializes Unique pointers to the different systems for communications
		****************************************************************************/
		void Init()
		{
			mComponentManager = std::make_unique<ComponentManager>();
			mEntityManager = std::make_unique<EntityManager>();
			mSystemManager = std::make_unique<SystemManager>();
		}

		// Entity methods

		/*!*************************************************************************
		Pulls a new entity ID from queue and return it
		****************************************************************************/
		Entity CreateEntity(bool isPrefab=false)
		{
			if (p_MemoryManager.IsEnabled() == false) return mEntityManager->CreateEntity(isPrefab);

			Entity id = mEntityManager->CreateEntity(isPrefab);
			void* slot = p_MemoryManager.Allocate(sizeof(Entity));
			mEntityMemory[id] = slot;
			if (slot)
			{
				*static_cast<Entity*>(slot) = id;
			}
			return id;
		}

		/*!*************************************************************************
		Destroys the Entity ID and push it back into the back of the queue
		****************************************************************************/
		void DestroyEntity(Entity entity)
		{
			mEntityManager->DestroyEntity(entity);
			if (p_MemoryManager.IsEnabled())
			{
				if (mEntityMemory[entity])
				{
					p_MemoryManager.Free(mEntityMemory[entity]);
					mEntityMemory[entity] = nullptr;
				}
			}

			mComponentManager->EntityDestroyed(entity);
			mSystemManager->EntityDestroyed(entity);
		}

		/*!*************************************************************************
		Clears all component data from all component arrays
		****************************************************************************/
		void ClearAllComponents()
		{
			mComponentManager->ClearAllComponents(mEntityManager->GetPrefabBitset());
		}

		/*!*************************************************************************
		Clones the Entity and all its available components
		****************************************************************************/
		Entity CloneEntity(Entity entity,bool isPrefab=false)
		{
			Entity newEntity;
			if (p_MemoryManager.IsEnabled())
			{
				newEntity = CreateEntity(isPrefab);
			}
			else
			{
				newEntity = mEntityManager->CreateEntity(isPrefab);
			}
			
			mEntityManager->SetSignature(newEntity, GetEntitySignature(entity));
			for (ComponentType i = 0; i < GetTotalRegisteredComponents(); ++i)
			{
				if (GetEntitySignature(newEntity).test(i))
				{
					mComponentManager->GetComponentArrayFromType(i)->CopyComponent(entity, newEntity);
				}
			}

			if (!isPrefab)
			{
				auto signature = mEntityManager->GetSignature(newEntity);
				mSystemManager->EntitySignatureChanged(newEntity, signature);
			}


			return newEntity;
		}

		/*!*************************************************************************
		Retrieves the entity's signature
		****************************************************************************/
		Signature GetEntitySignature(Entity entity)
		{
			return mEntityManager->GetSignature(entity);
		}

		/*!*************************************************************************
		Sets the Entity's signature for cloning
		****************************************************************************/
		void SetEntitySignature(Entity entity, Signature signature)
		{
			mEntityManager->SetSignature(entity, signature);
		}

		/*!*************************************************************************
		Returns the number of live entities
		****************************************************************************/
		Entity GetTotalSceneEntities()
		{
			return mEntityManager->GetTotalSceneEntities();
		}

		/*!*************************************************************************
		Returns the number of live entities
		****************************************************************************/
		Entity GetAliveEntities()
		{
			return static_cast<Entity>(mEntityManager->GetAliveEntities().size());
		}

		std::set<Entity> GetAliveEntitiesSet()
		{
			return mEntityManager->GetAliveEntities();
		}

		/*!*************************************************************************
		Sets the total entities for Scene build 
		****************************************************************************/
		void SetTotalEntitiesForWorldBuild(Entity entity)
		{
			mEntityManager->SetTotalEntitiesForWorld(entity);
		}

		/*!*************************************************************************
		Resets all the entities for Scene build
		****************************************************************************/
		void ResetEntities(bool withPrefabs=false)
		{
			auto aliveset = mEntityManager->GetAliveEntities();
			std::bitset<MAX_ENTITIES> bitset= mEntityManager->GetPrefabBitset();
			
			for (const auto& iter : aliveset)
			{
				if (withPrefabs)
				{
					DestroyEntity(iter);
				}
				else if (!bitset[iter]) 
				{
					DestroyEntity(iter);
				}
				
			}
			
		}

		/*!*************************************************************************
		Primes the memory manager with the expected number of entities for scene load
		****************************************************************************/
		void PrepareMemoryForEntities(Entity expected)
		{
			p_MemoryManager.Prime(static_cast<std::size_t>(expected));
			for (std::size_t i = 0; i < MAX_ENTITIES; ++i)
			{
				mEntityMemory[i] = nullptr;
			}
		}

		void ResetAllEntitySignatures()
		{
			mEntityManager->ResetAllEntitySignatures();
		}

		// Component methods
		/*!*************************************************************************
		Registers Component for use in ECS
		****************************************************************************/
		template<typename T>
		void RegisterComponent()
		{
			mComponentManager->RegisterComponent<T>();
		}

		/*!*************************************************************************
		Adds Component to Component Array with the assigned Entity
		****************************************************************************/
		template<typename T>
		void AddComponent(Entity entity, T component)
		{
			component.SetComponentEntityID(entity);
			mComponentManager->AddComponent<T>(entity, component);
			auto signature = mEntityManager->GetSignature(entity);
			signature.set(mComponentManager->GetComponentType<T>(), true);
			mEntityManager->SetSignature(entity, signature);

			if (!mEntityManager->GetPrefabBitset()[entity])
			{
				mSystemManager->EntitySignatureChanged(entity, signature);
			}

		}

		/*!*************************************************************************
		Remove Component and data from selected entity
		****************************************************************************/
		template<typename T>
		void RemoveComponent(Entity entity)
		{
			mComponentManager->RemoveComponent<T>(entity);

			auto signature = mEntityManager->GetSignature(entity);
			signature.set(mComponentManager->GetComponentType<T>(), false);
			mEntityManager->SetSignature(entity, signature);
			if (!mEntityManager->GetPrefabBitset()[entity])
			{
				mSystemManager->EntitySignatureChanged(entity, signature);
			}

		}

		/*!*************************************************************************
		Retrieve Component Data from the following component and selected entity
		****************************************************************************/
		template<typename T>
		T& GetComponent(Entity entity)
		{
			return mComponentManager->GetComponent<T>(entity);
		}

		/*!*************************************************************************
		Retrieves Component signature type
		****************************************************************************/
		template<typename T>
		ComponentType GetComponentType()
		{
			return mComponentManager->GetComponentType<T>();
		}

		/*!*************************************************************************
		Get name of component based on component type
		****************************************************************************/
		std::string GetComponentTypeName(ComponentType Type)
		{
			return mComponentManager->GetComponentTypeName(Type);
		}

		/*!*************************************************************************
		Checks if component exist
		****************************************************************************/
		template<typename T>
		bool HaveComponent(Entity entity)
		{
			return mComponentManager->HaveComponent<T>(entity);
		}

		/*!*************************************************************************
		Returns array containing the EntityToIndexMap
		****************************************************************************/
		std::array<size_t, MAX_ENTITIES>& GetEntityToIndexMapECS(ComponentType Type)
		{
			return mComponentManager->GetEntityToIndexMap(Type);
		}

		/*!*************************************************************************
		Returns array containing the IndexToEntityMap
		****************************************************************************/
		std::array<Entity, MAX_ENTITIES>& GetIndexToEntityMapECS(ComponentType Type)
		{
			return mComponentManager->GetIndexToEntityMap(Type);
		}

		/*!*************************************************************************
		Returns total number of Components registered in the ECS
		****************************************************************************/
		const ComponentType GetTotalRegisteredComponents()
		{
			return mComponentManager->GetTotalRegisteredComponents();
		}

		/*!*************************************************************************
		Returns the size of Array based on ComponentType
		****************************************************************************/
		const size_t GetEntitySize(ComponentType Type)
		{
			return mComponentManager->GetEntitySize(Type);
		}

		/*!*************************************************************************
		Clears the Mappings for Scene Build
		****************************************************************************/
		void ClearArrayForWorldBuild(ComponentType Type)
		{
			mComponentManager->ClearArrayForWorldBuild(Type);
		}

		/*!*************************************************************************
		Returns ComponentArray Based on ComponentType
		****************************************************************************/
		std::shared_ptr<IComponentArray> GetComponentArrayFromType(ComponentType Type)
		{
			return mComponentManager->GetComponentArrayFromType(Type);
		}


		// System methods
		/*!*************************************************************************
		Registers the System for use in the ECS
		****************************************************************************/
		template<typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			return mSystemManager->RegisterSystem<T>();
		}

		/*!*************************************************************************
		Sets the System signature for which component the system shall look out for
		****************************************************************************/
		template<typename T>
		void SetSystemSignature(Signature signature)
		{
			mSystemManager->SetSignature<T>(signature);
		}

	private:
		// Unique Pointer to ComponentManager
		std::unique_ptr<ComponentManager> mComponentManager;
		// Unique Pointer to EntityManager
		std::unique_ptr<EntityManager> mEntityManager;
		// Unique Pointer to SystemManager
		std::unique_ptr<SystemManager> mSystemManager;
		// Unique Pointer to ECS instance
		inline static std::unique_ptr<ECS> m_instance;
		// Memory blocks associated with entities when using the custom allocator
		void* mEntityMemory[MAX_ENTITIES]{};
	};
}
