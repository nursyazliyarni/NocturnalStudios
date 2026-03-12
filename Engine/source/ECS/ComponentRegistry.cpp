/*!------------------------------------------------------------------------------------------------
	File: ComponentRegistry.cpp
	Project: NocturnalStudio

	Brief:
		:3 singular ecs helper function

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- .

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "ECS/ComponentRegistry.h"

namespace NocturnalEngine{
    
	ECS& ComponentRegistry::GetECS() {
		return p_ecs;
	}

}
