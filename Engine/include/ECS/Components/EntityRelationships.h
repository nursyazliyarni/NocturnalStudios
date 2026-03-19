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
#include "IComponent.h"
#include "glm/glm.hpp"
namespace NocturnalEngine {
	namespace ParentingSystem {
		void SetParent(Entity child, Entity parent);
		
		void RemoveParent(Entity child);

		void RemoveChildren(Entity parent);

		void DestroyHierarchy(Entity e);

		void RemapRelationships(const std::unordered_map<Entity, Entity>& idRemap);

		Entity Remap(Entity e, const std::unordered_map<Entity, Entity>& idRemap);

		Entity CloneEntity(Entity source, bool isPrefab=false);

		Entity CloneHierarchy(Entity source, Entity newParent, bool isPrefab=false);
	}
	class Relationship : public IComponent
	{
	public:

		Relationship();
		~Relationship() = default;
		virtual bool Deserialize(const rapidjson::Value& obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;
		bool OnCompare(const IComponent& other) const override;
		bool ToEditor()override;

		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

		// Getters/Setters
		Entity GetParent()      const { return parent; }
		Entity GetFirstChild()  const { return firstChild; }
		Entity GetNextSibling() const { return nextSibling; }
		Entity GetPrevSibling() const { return prevSibling; }
		uint32_t& GetChildCount() { return childCount; }
		bool IsParent()	const { return isParent; }
		bool IsChild()	const { return isChild; }

		void EntityIsParent(bool p) { isParent = p; }
		void EntityIsChild(bool c) { isChild = c; }
		void SetParent(Entity e) { parent = e; }
		void SetFirstChild(Entity e) { firstChild = e; }
		void SetNextSibling(Entity e) { nextSibling = e; }
		void SetPrevSibling(Entity e) { prevSibling = e; }
	private:
		bool isParent{ false }, isChild{ false };
		Entity parent{ MAX_ENTITIES };
		Entity firstChild{ MAX_ENTITIES };
		Entity nextSibling{ MAX_ENTITIES };
		Entity prevSibling{ MAX_ENTITIES };
		uint32_t childCount{ 0 };
	};
}
