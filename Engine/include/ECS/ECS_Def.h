#ifndef ECS_DEF_H
#define ECS_DEF_H

/*!------------------------------------------------------------------------------------------------
	File: ECS_Def.h
	Project: NocturnalStudio

	This file contains implementation details of ECS.h and it provides a nice public API without
	implementation details exposed in ECS.h

	Primary Author:
		Lau Xiang Yang (100%)
		- Created the file and implemented the ECS class member functions


	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>
#include <functional>
#include <cassert>
#include <any>
#include <limits>

/**
 * @file ECS_Def.h
 * @brief Fundamental typedefs, constants and small helper types used by the ECS.
 *
 * This header exposes the lightweight building blocks used throughout the ECS:
 * - strongly-typed aliases for IDs,
 * - sentinel constants,
 * - small POD structs that hold metadata,
 * - and the Family type-id generator used to obtain a unique ComponentID per C++ type.
 *
 * Usage examples:
 *   ComponentID cid = Family::getID<Transform>(); // unique id for Transform type
 *   EntityID e  = ecs.createEntity("Player");    // use EntityID values with ECS
 */

using ComponentID = std::size_t;
using SceneID     = std::size_t; // added by iwa
using LayerID     = std::size_t; // added by iwa
using EntityID    = std::size_t;
using SystemID    = std::size_t;

/**
 * @brief Special sentinel values for invalid or unselected IDs.
 *
 * These constants should be used when representing "no entity" or "no component".
 *
 * Usage:
 *   if (myEntity == INVALID_ENTITY_ID) { /* handle invalid *\/ }
 */
constexpr EntityID    INVALID_ENTITY_ID    = std::numeric_limits<std::size_t>::max();
constexpr ComponentID INVALID_COMPONENT_ID = std::numeric_limits<std::size_t>::max();
constexpr EntityID    NO_ENTITY_SELECTED   = std::numeric_limits<std::size_t>::max();

/**
 * @brief Lightweight wrapper representing an entity handle.
 *
 * Contains only the numeric `id`. Useful when passing small, explicit entity
 * objects around instead of raw integers.
 *
 * Usage:
 *   Entity e{ .id = someEntityID };
 */
struct Entity {
	EntityID id;
};

/**
 * @brief Metadata stored per-entity by the ECS.
 *
 * - `componentIDs` lists the ComponentID values currently attached to the entity.
 * - `name` stores an optional human-readable name for editor/debug purposes.
 *
 * Usage:
 *   EntityMetadata meta;
 *   meta.name = "Player";
 *   meta.componentIDs.insert(Family::getID<Transform>());
 */
struct EntityMetadata {
	std::set<ComponentID> componentIDs{};
	std::string name{};
};

/**
 * @brief Small record describing a registered component type.
 *
 * - `componentID` is the unique ID for the component type.
 * - `size` is the current number of component instances stored for that type.
 *
 * `clone()` returns a copy of the ComponentData (convenience helper).
 *
 * Usage:
 *   ComponentData cd{ Family::getID<Transform>(), 10 };
 *   ComponentData copy = cd.clone();
 */
struct ComponentData {
	ComponentID componentID;
	std::size_t size;

	ComponentData clone() const {
		return { componentID, size };
	}
};


/**
 * @brief Descriptor used by view/query systems.
 *
 * - `index` typically refers to an index into an internal registry of views.
 * - `componentIDs` lists component types required by the view.
 *
 * Usage:
 *   ViewData v{};
 *   v.index = 0;
 *   v.componentIDs.push_back(Family::getID<Transform>());
 */
struct ViewData {
	std::size_t index;
	std::vector<ComponentID> componentIDs{};
};

/**
 * @brief Family type-id generator.
 *
 * Family provides a compact way to obtain a unique numeric identifier for a
 * C++ type at runtime (but generated at compile-time per type). Each distinct
 * template instantiation of `Family::getID<T>()` returns a unique ComponentID.
 *
 * Implementation notes:
 * - `identifier()` holds an internal static counter and returns a new id on
 *   each call. It is private; callers should use `getID<T>()`.
 *
 * Usage:
 *   // Retrieves a stable, unique id for the Transform type:
 *   ComponentID transformID = Family::getID<Transform>();
 *
 *   // Different types produce different ids:
 *   assert(Family::getID<Transform>() != Family::getID<ImageRenderer>());
 */
class Family {
private:
	/**
	 * @brief Returns a fresh incremental id (internal).
	 *
	 * Each invocation yields a new value using an internal static counter.
	 * This function is intentionally private; `getID<T>()` is the public API.
	 */
	static std::size_t identifier() {
		static std::size_t lastID = 0;
		return lastID++;
	}

public:
	/**
	 * @brief Get a unique id for the template type parameter.
	 *
	 * The returned id is unique per distinct `T` and stable for the lifetime
	 * of the process. Internally it calls `identifier()` once per distinct
	 * T and caches the result in a static variable.
	 *
	 * Template usage example:
	 *   ComponentID cid = Family::getID<Transform>();
	 */
	template<typename>
	static std::size_t getID() {
		static const std::size_t value = identifier();
		return value;
	}
};




#endif // ECS_DEF_H