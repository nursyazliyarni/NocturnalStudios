/*!------------------------------------------------------------------------------------------------
	File: EntityRelationships.h
	Project: NocturnalStudio

	Brief:
		This file contains class and functions used by the HUD component

	Primary Author:
		 LAU XIANG YANG (100%)
		- Created the Relationship.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#pragma once
#include "ECS.h"

#define p_Heirarchy NocturnalEngine::ParentingSystem::GetInstance()
namespace NocturnalEngine {
	class PrefabManager; 
	class ParentingSystem {
	public:
		static ParentingSystem& GetInstance() {
			static ParentingSystem instance;
			return instance;
		}
		struct Relationship {
			bool isParent{ false }, isChild{ false };
			bool isPrefab{ false };
			Entity parent{ MAX_ENTITIES };
			Entity firstChild{ MAX_ENTITIES };
			Entity nextSibling{ MAX_ENTITIES };
			Entity prevSibling{ MAX_ENTITIES };
			uint32_t childCount{ 0 };
		};
		void SetParent(Entity child, Entity parent,bool isPrefab=false,bool isStartup=false);

		void RemoveParent(Entity child);

		void RemoveChildren(Entity parent);

		void DestroyHierarchy(Entity e);

		void RemapRelationships(const std::unordered_map<Entity, Entity>& idRemap);

		Entity Remap(Entity e, const std::unordered_map<Entity, Entity>& idRemap);

		Entity CloneEntity(Entity source, bool isPrefab = false);

		Entity CloneHierarchy(Entity source, Entity newParent, bool isPrefab = false);

		bool IsChild(Entity e)  {
			if (!EntityRelationshipExists(e))return false;

			return entityRelationships[e].isChild;
		}

		bool IsParent(Entity e)  {
			if (!EntityRelationshipExists(e))return false;

			return entityRelationships[e].isParent;
		}

		void CleanupEntity(Entity e)
		{
			RemoveParent(e);
			RemoveChildren(e);
			entityRelationships.erase(e);
		}
	
		void EnsureRelationshipExists(Entity e, bool isPrefab = false)
		{
			if (!EntityRelationshipExists(e))
			{
				Relationship blank{};
				blank.isPrefab = isPrefab;
				entityRelationships.emplace(e, blank);
			}
		}

		std::vector<Entity> GetChildren(Entity e) const {
			if (!EntityRelationshipExists(e)) return {};

			std::vector<Entity> children;
			Entity child = entityRelationships.at(e).firstChild;

			while (child != MAX_ENTITIES)
			{
				children.push_back(child);
				if (!EntityRelationshipExists(child)) break;
				child = entityRelationships.at(child).nextSibling;
			}

			return children;
		}

		Entity GetParent(Entity e) const {
			if (!EntityRelationshipExists(e)) return MAX_ENTITIES;

			return entityRelationships.at(e).parent;
		}

		bool IsPrefabEntity(Entity e) const {
			if (!EntityRelationshipExists(e)) return false;
			return entityRelationships.at(e).isPrefab;
		}
		bool EntityRelationshipExists(Entity e) const {
			return entityRelationships.contains(e);
		}
		void ClearSceneEntities();
	private:


		std::unordered_map<Entity, Relationship> entityRelationships{};
	};
	
}
