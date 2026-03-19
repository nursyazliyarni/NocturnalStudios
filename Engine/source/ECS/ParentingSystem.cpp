/*!------------------------------------------------------------------------------------------------
	File: EntityRelationships.cpp
	Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize HUD components for ECS.

	Primary Author:
		 LAU XIANG YANG (100%)
		- Created the HUD component class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/ParentingSystem.h"
#include "ECS/ECS.h"
#include <imgui.h>

namespace NocturnalEngine {


		void ParentingSystem::SetParent(Entity child, Entity parent, bool isPrefab)
		{
			Relationship blank {};
			if (!EntityRelationshipExists(child))entityRelationships.emplace(child,blank);
			if (!EntityRelationshipExists(parent))entityRelationships.emplace(parent, blank);

			Relationship& childRel = entityRelationships[child];
			Relationship& parentRel = entityRelationships[parent];

			entityRelationships[child].isPrefab = isPrefab;
			entityRelationships[parent].isPrefab = isPrefab;

			// Detach from old parent first
			if (childRel.parent != MAX_ENTITIES)
				RemoveParent(child);

			childRel.parent=parent;

			// Insert at front of parent's child linked list
			Entity oldFirst = parentRel.firstChild;
			if (EntityRelationshipExists(oldFirst))
			{
				Relationship& oldFirstRel = entityRelationships[oldFirst];
				oldFirstRel.prevSibling = child;
				childRel.nextSibling = oldFirst;
			}

			parentRel.firstChild=child;
			childRel.isChild = true;        
			parentRel.isParent = true;      
			parentRel.childCount++;
		}

		// Detach child from its current parent
		void ParentingSystem::RemoveParent(Entity child)
		{
			if (!EntityRelationshipExists(child)) return;

			Relationship& childRel = entityRelationships[child];
			Entity parent = childRel.parent;

			if (parent == MAX_ENTITIES) return;
			if (!EntityRelationshipExists(parent)) return;

			Relationship& parentRel = entityRelationships[parent];

			Entity prev = childRel.prevSibling;
			Entity next = childRel.nextSibling;

			// Rewire linked list around this child
			if (prev != MAX_ENTITIES&& EntityRelationshipExists(prev))
				entityRelationships[prev].nextSibling=next;
			else
				parentRel.firstChild =next; // was the first child

			if (next != MAX_ENTITIES && EntityRelationshipExists(next))
				entityRelationships[next].prevSibling=prev;

			// Clean up child's own links
			childRel.parent=MAX_ENTITIES;
			childRel.nextSibling=MAX_ENTITIES;
			childRel.prevSibling=MAX_ENTITIES;
			childRel.isChild = false;                                    
			parentRel.childCount--;
			if (parentRel.childCount == 0) parentRel.isParent = false;
		}

		void ParentingSystem::RemoveChildren(Entity parent) 
		{
			if (!EntityRelationshipExists(parent)) return;

			Relationship& parentRel = entityRelationships[parent];
			Entity firstChild = parentRel.firstChild;

			while (firstChild != MAX_ENTITIES)
			{
				if (!EntityRelationshipExists(firstChild)) {
					firstChild = MAX_ENTITIES;
					continue;
				}
				Entity next = entityRelationships[firstChild].nextSibling;
				RemoveParent(firstChild); 
				firstChild = next;
			}
		}

		void ParentingSystem::DestroyHierarchy(Entity e)
		{

			if (EntityRelationshipExists(e))
			{
				Relationship& rel = entityRelationships[e];
				Entity child = rel.firstChild;

				while (child != MAX_ENTITIES)
				{
					if (!EntityRelationshipExists(child)) {
						child = MAX_ENTITIES;
						continue;
					}
					Entity next = entityRelationships[child].nextSibling;
					DestroyHierarchy(child);
					child = next;
				}

				CleanupEntity(e);
			}

			// Always destroy the ECS entity regardless of relationship existence
			p_ecs.DestroyEntity(e);

		}

		void ParentingSystem::RemapRelationships(const std::unordered_map<Entity, Entity>& idRemap) {
			for (const std::pair<Entity,Entity>& e : idRemap)
			{
				if (!EntityRelationshipExists(e.second)) continue;

				Relationship& rel = entityRelationships[e.second];

				rel.parent=Remap(rel.parent, idRemap);
				rel.firstChild=Remap(rel.firstChild, idRemap);
				rel.nextSibling=Remap(rel.nextSibling, idRemap);
				rel.prevSibling=Remap(rel.prevSibling, idRemap);
			}
		}

		Entity ParentingSystem::Remap(Entity e, const std::unordered_map<Entity, Entity>& idRemap) {
			auto it = idRemap.find(e);
			return it != idRemap.end() ? it->second : MAX_ENTITIES;
		}

		Entity ParentingSystem::CloneHierarchy(Entity source, Entity newParent, bool isPrefab)
		{
			Entity newEntity = p_ecs.CloneEntity(source, isPrefab);

			// Always create relationship entry, not just when source has one
			Relationship blank{};
			blank.isPrefab = isPrefab;
			entityRelationships.emplace(newEntity, blank);

			if (newParent != MAX_ENTITIES)
				SetParent(newEntity, newParent, isPrefab);

			// Recurse into children
			if (EntityRelationshipExists(source))
			{
				Relationship& srcRel = entityRelationships[source];
				Entity child = srcRel.firstChild;

				while (child != MAX_ENTITIES)
				{
					Entity nextChild = MAX_ENTITIES;
					if (EntityRelationshipExists(child))
						nextChild = entityRelationships[child].nextSibling;

					CloneHierarchy(child, newEntity, isPrefab);
					child = nextChild;
				}
			}

			return newEntity;
		}

		// Entry point — call this to clone a root entity + all its children
		Entity ParentingSystem::CloneEntity(Entity source, bool isPrefab)
		{
			return CloneHierarchy(source, MAX_ENTITIES,isPrefab);
		}
	
		void ParentingSystem::ClearSceneEntities()
		{
			// Collect non-prefab roots first — avoids modifying map while iterating
			std::vector<Entity> roots;
			for (const std::pair<Entity,Relationship>& e : entityRelationships)
			{
				if (!e.second.isPrefab && e.second.parent == MAX_ENTITIES)
					roots.push_back(e.first);
			}

			for (Entity root : roots)
				DestroyHierarchy(root);
		}
	
}
