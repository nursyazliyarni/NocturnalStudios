/*!------------------------------------------------------------------------------------------------
    File: EntityManager.h
    Project: NocturnalStudio

	Brief:
		This file contains all the functions for distributing unique IDs to
		all entities created, it does this using a container(queue) to store all
		available entities allowed for the game engine to use
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the EntityManager class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "ECS/Types.h"
#include "Core/Log.h"

namespace NocturnalEngine
{
	class EntityManager
	{
	public:
		
		/*!*************************************************************************
		Initializes the queue for for all available entity IDs
		****************************************************************************/
		EntityManager()
		{
			// Initialize the queue with all possible entity IDs up to the max number of entities
			for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
			{
				mAvailableEntities.push(entity);
			}
		}

		/*!*************************************************************************
		Grabs a new id from the queue and return it
		****************************************************************************/
		Entity CreateEntity(bool isPrefab=false)
		{
			assert(mTotalEntityCount < MAX_ENTITIES && "Too many entities in existence.");
			// Take an ID from the front of the queue
			Entity id = mAvailableEntities.top();
			mAvailableEntities.pop();	//Removes elements from the front of the queue
			mIsPrefab[id] = isPrefab;

			if (!isPrefab)
			{
				mAliveEntities.insert(id);
				++mLivingEntityCount;	//Dictates how many entities are alive
			}
			else 
			{
				mPrefabEntities.insert(id);
				++mPrefabEntityCount;	//Dictates how many entities are alive
			}
			++mTotalEntityCount;

			return id;
		}

		/*!*************************************************************************
		Destroy the entities id and push it to end of queue
		****************************************************************************/
		void DestroyEntity(Entity entity)
		{
			assert(entity < MAX_ENTITIES && "Entity out of range.");

			// Put the destroyed ID at the back of the queue
			mSignatures[entity].reset();
			mAvailableEntities.push(entity);
			if (mIsPrefab[entity])
			{
				mPrefabEntities.erase(entity);
				--mPrefabEntityCount;
				mIsPrefab[entity] = false;
			}
			else {
				mAliveEntities.erase(entity);
				--mLivingEntityCount;
			}
			--mTotalEntityCount;
		}

		/*!*************************************************************************
		Parse the entity's signature into an array of Signatures
		****************************************************************************/
		void SetSignature(Entity entity, Signature signature)
		{
			assert(entity < MAX_ENTITIES && "Entity out of range.");

			// Put this entity's signature into the array
			mSignatures[entity] = signature;
		}

		/*!*************************************************************************
		Get the Signature of the entity
		****************************************************************************/
		Signature GetSignature(Entity entity)
		{
			assert(entity < MAX_ENTITIES && "Entity out of range.");

			// Get this entity's signature from the array
			return mSignatures[entity];
		}


		/*!*************************************************************************
		Returns the Total Entities in existence (both destroyed + alive)
		****************************************************************************/
		Entity GetTotalSceneEntities()
		{
			return mLivingEntityCount;
		}

		/*!*************************************************************************
		Builds the Total entities for Scene Build
		****************************************************************************/
		void SetTotalEntitiesForWorld(Entity entity)
		{
			while(mAliveEntities.size() < entity)
			{
				CreateEntity();
			}
		}

		/*!*************************************************************************
		Returns the live entities
		****************************************************************************/
		const std::set<Entity> GetAliveEntities()
		{
			return mAliveEntities;
		}

		const std::bitset<MAX_ENTITIES>& GetPrefabBitset() const {
			return mIsPrefab;
		}

		std::array<Signature, MAX_ENTITIES>& GetEntitySignatureArray()
		{
			return mSignatures;
		}

		void ResetAllEntitySignatures()
		{
			for (Entity e = 0; e < MAX_ENTITIES; e++)
				if (!mIsPrefab[e])
					mSignatures[e] = 0;
		}

		
	private:

		// Queue of unused entity IDs
		std::priority_queue<Entity, std::vector<Entity>, std::greater<Entity>> mAvailableEntities{};

		// Array of signatures where the index corresponds to the entity ID
		std::array<Signature, MAX_ENTITIES> mSignatures{};

		std::bitset<MAX_ENTITIES> mIsPrefab{};

		// Total living entities - used to keep limits on how many exist
		Entity mLivingEntityCount{}, mPrefabEntityCount{}, mTotalEntityCount{};

		// Alive entities that are still in use
		std::set<Entity> mAliveEntities;

		//Storage for prefab entities, arent cleared out when scenes are reset.
		std::set<Entity> mPrefabEntities;
	};
}