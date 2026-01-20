/*!------------------------------------------------------------------------------------------------
    File: Picker.h
    Project: NocturnalStudio

	Brief:
		This file contains the declaration for the Picker struct used for 
		object picking in the level editor.
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Picker.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Graphics/Camera2D.h"
namespace NocturnalEngine {

	struct AABB
	{
		glm::vec2 min;
		glm::vec2 max;
	};

	struct Picker
	{
		static int Pick(Camera2D* camera, std::multimap<float, Transform*> scene);
	};
}