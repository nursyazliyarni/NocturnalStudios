#ifndef ECS_COMPONENT_H
#define ECS_COMPONENT_H

/*!------------------------------------------------------------------------------------------------
	File: ECS.h
	Project: NocturnalStudio

	This file contains the struct definition of the different component types that can be used in 
	the ECS.

	Primary Author:
		
		

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include <string>
#include <type_traits>

#include "../include/math/Matrix.h"
#include "../include/math/Vector.h"

//-------------------------- Add new ECS component type here --------------------------//

struct Transform;
struct Image;

//-------------------------- ECS Component Definitions --------------------------//

struct Transform {
	Vector2D position;
	Vector2D scale;
	Vector2D lastPosition = {};
	Vector2D lastScale = {};

	float rotation;
	float height;
	float lastRotation = rotation;
	float lastHeight = height;

	enum class CoordinateSpace {
		World,
		Viewport
	};

	CoordinateSpace space = CoordinateSpace::World;
	bool isVisible = true;
};

struct Image {
	int textureID = -1; // Default to -1 (no texture)
	float opacity = 1.0f; // Default to fully opaque
	bool isVisible = true;

	Vector4D colorTint = { 1.0f, 1.0f, 1.0f, 1.0f }; 
	Vector4D colorToAdd = {};
};

#endif // COMPONENT_H