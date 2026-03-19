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
#include "ECS/Components/EntityRelationships.h"
#include "ECS/ECS.h"
#include <imgui.h>

namespace NocturnalEngine {

	namespace ParentingSystem {
		void SetParent(Entity child, Entity parent)
		{
			if (!p_ecs.HaveComponent<Relationship>(child))p_ecs.AddComponent<Relationship>(child, {});
			if (!p_ecs.HaveComponent<Relationship>(parent))p_ecs.AddComponent<Relationship>(parent, {});

			Relationship& childRel = p_ecs.GetComponent<Relationship>(child);
			Relationship& parentRel = p_ecs.GetComponent<Relationship>(parent);

			// Detach from old parent first
			if (childRel.GetParent() != MAX_ENTITIES)
				RemoveParent(child);

			childRel.SetParent(parent);

			// Insert at front of parent's child linked list
			Entity oldFirst = parentRel.GetFirstChild();
			if (oldFirst != MAX_ENTITIES)
			{
				auto& oldFirstRel = p_ecs.GetComponent<Relationship>(oldFirst);
				oldFirstRel.SetPrevSibling(child);
				childRel.SetNextSibling(oldFirst);
			}

			parentRel.SetFirstChild(child);
			parentRel.GetChildCount()++;
		}

		// Detach child from its current parent
		void RemoveParent(Entity child)
		{
			if (!p_ecs.HaveComponent<Relationship>(child)) return;

			Relationship& childRel = p_ecs.GetComponent<Relationship>(child);
			Entity parent = childRel.GetParent();

			if (parent == MAX_ENTITIES) return;
			if (!p_ecs.HaveComponent<Relationship>(parent)) return;

			Relationship& parentRel = p_ecs.GetComponent<Relationship>(parent);

			Entity prev = childRel.GetPrevSibling();
			Entity next = childRel.GetNextSibling();

			// Rewire linked list around this child
			if (prev != MAX_ENTITIES&& p_ecs.HaveComponent<Relationship>(prev))
				p_ecs.GetComponent<Relationship>(prev).SetNextSibling(next);
			else
				parentRel.SetFirstChild(next); // was the first child

			if (next != MAX_ENTITIES && p_ecs.HaveComponent<Relationship>(next))
				p_ecs.GetComponent<Relationship>(next).SetPrevSibling(prev);

			// Clean up child's own links
			childRel.SetParent(MAX_ENTITIES);
			childRel.SetNextSibling(MAX_ENTITIES);
			childRel.SetPrevSibling(MAX_ENTITIES);
			parentRel.GetChildCount()--;
		}

		void RemoveChildren(Entity parent) {
			if (!p_ecs.HaveComponent<Relationship>(parent)) return;

			Relationship& parentRel = p_ecs.GetComponent<Relationship>(parent);
			Entity firstChild = parentRel.GetFirstChild();

			while (firstChild != MAX_ENTITIES)
			{
				if (!p_ecs.HaveComponent<Relationship>(firstChild)) {
					firstChild = MAX_ENTITIES;
					continue;
				}
				Entity next = p_ecs.GetComponent<Relationship>(firstChild).GetNextSibling();
				RemoveParent(firstChild); 
				firstChild = next;
			}
		}

		void DestroyHierarchy(Entity e)
		{
			if (!p_ecs.HaveComponent<Relationship>(e)) return;

			Relationship& rel = p_ecs.GetComponent<Relationship>(e);
			Entity child = rel.GetFirstChild();

			while (child != MAX_ENTITIES)
			{
				if (!p_ecs.HaveComponent<Relationship>(child)) {
					child = MAX_ENTITIES; 
					continue;
				}
				Entity next = p_ecs.GetComponent<Relationship>(child).GetNextSibling();
				DestroyHierarchy(child); // recurse
				child = next;
			}

			p_ecs.DestroyEntity(e);
		}

		void RemapRelationships(const std::unordered_map<Entity, Entity>& idRemap) {
			for (const std::pair<Entity,Entity>& e : idRemap)
			{
				if (!p_ecs.HaveComponent<Relationship>(e.second)) continue;

				Relationship& rel = p_ecs.GetComponent<Relationship>(e.second);

				rel.SetParent(Remap(rel.GetParent(), idRemap));
				rel.SetFirstChild(Remap(rel.GetFirstChild(), idRemap));
				rel.SetNextSibling(Remap(rel.GetNextSibling(), idRemap));
				rel.SetPrevSibling(Remap(rel.GetPrevSibling(), idRemap));
			}
		}

		Entity Remap(Entity e, const std::unordered_map<Entity, Entity>& idRemap) {
			auto it = idRemap.find(e);
			return it != idRemap.end() ? it->second : MAX_ENTITIES;
		}

		Entity CloneHierarchy(Entity source, Entity newParent, bool isPrefab)
		{
			// Clone the entity itself — copy all components
			Entity newEntity = p_ecs.CloneEntity(source,isPrefab);

			// Reset the cloned Relationship to a clean state — the copied one
			// still points to source's parent/siblings which is wrong
			if (p_ecs.HaveComponent<Relationship>(newEntity))
			{
				Relationship& rel = p_ecs.GetComponent<Relationship>(newEntity);
				rel.SetParent(MAX_ENTITIES);
				rel.SetFirstChild(MAX_ENTITIES);
				rel.SetNextSibling(MAX_ENTITIES);
				rel.SetPrevSibling(MAX_ENTITIES);
				rel.GetChildCount() = 0;
			}

			// If this clone has a parent, wire it up
			if (newParent != MAX_ENTITIES)
				SetParent(newEntity, newParent);

			// Recurse into children
			if (p_ecs.HaveComponent<Relationship>(source))
			{
				Relationship& srcRel = p_ecs.GetComponent<Relationship>(source);
				Entity child = srcRel.GetFirstChild();

				while (child != MAX_ENTITIES)
				{
					Entity nextChild = MAX_ENTITIES;
					if (p_ecs.HaveComponent<Relationship>(child))
						nextChild = p_ecs.GetComponent<Relationship>(child).GetNextSibling();

					// Clone child and attach to our new entity
					CloneHierarchy(child, newEntity,isPrefab);

					child = nextChild;
				}
			}

			return newEntity;
		}

		// Entry point — call this to clone a root entity + all its children
		Entity CloneEntity(Entity source, bool isPrefab)
		{
			return CloneHierarchy(source, MAX_ENTITIES,isPrefab);
		}
	}

	/*!*************************************************************************
	Constructor for Relationship component
	****************************************************************************/
	Relationship::Relationship() {}

	bool Relationship::ToEditor() {
	
		bool changed{ false };
		if (ImGui::CollapsingHeader("Relationship", ImGuiTreeNodeFlags_None))
		{
			bool hasParent = parent != MAX_ENTITIES;
			bool hasChildren = childCount > 0;

			// Read-only info display
			ImGui::BeginDisabled();
			if (hasParent)
				ImGui::Text("Parent:       %u", parent);
			if (hasChildren)
			{
				ImGui::Text("First Child:  %u", firstChild);
				ImGui::Text("Child Count:  %u", childCount);
			}
			if (hasParent)
			{
				ImGui::Text("Next Sibling: %s", nextSibling == MAX_ENTITIES ? "None" : std::to_string(nextSibling).c_str());
				ImGui::Text("Prev Sibling: %s", prevSibling == MAX_ENTITIES ? "None" : std::to_string(prevSibling).c_str());
			}
			ImGui::EndDisabled();

			// Show a hint if this entity is completely unconnected
			if (!hasParent && !hasChildren)
				ImGui::TextDisabled("No parent or children assigned.");

			ImGui::Separator();

			// Only show if actually has a parent
			if (hasParent)
			{
				if (ImGui::Button("Remove Parent"))
				{
					ParentingSystem::RemoveParent(entityID);
					changed = true;
				}
			}

			// Only show if actually has children
			if (hasChildren)
			{
				if (hasParent) ImGui::SameLine();
				if (ImGui::Button("Remove All Children"))
				{
					ParentingSystem::RemoveChildren(entityID);
					changed = true;
				}

				ImGui::Separator();
				if (ImGui::TreeNode("Children"))
				{
					Entity child = firstChild;
					while (child != MAX_ENTITIES)
					{
						ImGui::Text("Entity %u", child);
						ImGui::SameLine();
						ImGui::PushID((int)child);
						if (ImGui::SmallButton("Detach"))
						{
							ParentingSystem::RemoveParent(child);
							changed = true;
							ImGui::PopID();
							break;
						}
						ImGui::PopID();

						if (!p_ecs.HaveComponent<Relationship>(child)) break;
						child = p_ecs.GetComponent<Relationship>(child).GetNextSibling();
					}
					ImGui::TreePop();
				}
			}
		}

		return changed;
	}

	/*!*************************************************************************
	Derserialize Relationship component
	****************************************************************************/
	bool Relationship::Deserialize(const rapidjson::Value& obj)
	{
		if (obj.HasMember("IsParent"))    isParent = obj["IsParent"].GetBool();
		if (obj.HasMember("IsChild"))      isChild = obj["IsChild"].GetBool();
		if (obj.HasMember("Parent"))      parent = obj["Parent"].GetUint();
		if (obj.HasMember("FirstChild"))  firstChild = obj["FirstChild"].GetUint();
		if (obj.HasMember("NextSibling")) nextSibling = obj["NextSibling"].GetUint();
		if (obj.HasMember("PrevSibling")) prevSibling = obj["PrevSibling"].GetUint();
		if (obj.HasMember("ChildCount"))  childCount = obj["ChildCount"].GetUint();
		return true;
	}

	/*!*************************************************************************
	Serialize Relationship component
	****************************************************************************/
	bool Relationship::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		writer->Key("IsParent");	writer->Bool(isParent);
		writer->Key("IsChild");		writer->Bool(isChild);
		writer->Key("Parent");      writer->Uint(parent);
		writer->Key("FirstChild");  writer->Uint(firstChild);
		writer->Key("NextSibling"); writer->Uint(nextSibling);
		writer->Key("PrevSibling"); writer->Uint(prevSibling);
		writer->Key("ChildCount");  writer->Uint(childCount);
		return true;
	}

	bool Relationship::OnCompare(const IComponent& other) const {
		const Relationship& o = static_cast<const Relationship&>(other);
		return isParent == o.isParent
			&& isChild == o.isChild
			&& parent == o.parent
			&& firstChild == o.firstChild
			&& nextSibling == o.nextSibling
			&& prevSibling == o.prevSibling
			&& childCount == o.childCount;
	}
}
