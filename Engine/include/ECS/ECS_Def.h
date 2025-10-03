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

using ComponentID = std::size_t;
using EntityID = std::size_t;
using SystemID = std::size_t;

constexpr EntityID INVALID_ENTITY_ID = std::numeric_limits<std::size_t>::max();
constexpr ComponentID INVALID_COMPONENT_ID = std::numeric_limits<std::size_t>::max();
constexpr EntityID NO_ENTITY_SELECTED = std::numeric_limits<std::size_t>::max();

struct Entity {
	EntityID id;
};

struct EntityMetadata {
	std::set<ComponentID> componentIDs{};
	std::string name{};
};

struct ComponentData {
	ComponentID componentID;
	std::size_t size;

	ComponentData clone() const {
		return { componentID, size };
	}
};

struct ViewData {
	std::size_t index;
	std::vector<ComponentID> componentIDs{};
};

class Family {
private:
	static std::size_t identifier() {
		static std::size_t lastID = 0;
		return lastID++;
	}

public:
	template<typename>
	static std::size_t getID() {
		static const std::size_t value = identifier();
		return value;
	}
};


#endif // ECS_DEF_H