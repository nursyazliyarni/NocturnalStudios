/*!------------------------------------------------------------------------------------------------
	File: ComponentRegistry.h
	Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize name tag components for ECS.

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the draggable class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "ECS/ComponentRegistry.h"

namespace NocturnalEngine{
    
	ECS& ComponentRegistry::GetECS() {
		return p_ecs;
	}

}
