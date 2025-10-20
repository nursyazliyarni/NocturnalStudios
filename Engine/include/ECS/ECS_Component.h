#ifndef ECS_COMPONENT_H
#define ECS_COMPONENT_H

/*!------------------------------------------------------------------------------------------------
	File: ECS_Component.h
	Project: NocturnalStudio

	This file contains the struct definition of the different component types that can be used in 
	the ECS.

	Primary Author:
		Lau Xiang Yang (100%)
		- Created the file and implemented the ECS component structs
		

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include <string>
#include <type_traits>

#include "../include/math/Matrix.h"
#include "../include/math/Vector.h"

//-------------------------- Add new ECS component type here --------------------------//

struct Transform;
struct Renderer;
struct ImageRenderer;
struct CircleRenderer;
struct RectRenderer;
struct AABBCollider;
struct AnimalBehavior;

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

//renderer base struct
struct Renderer {
	float opacity = 1.0f; // Default to fully opaque
	bool isVisible = true;

	Vector4D colorTint = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4D colorToAdd = {};
};

struct ImageRenderer: public Renderer {
	int textureID = -1; // Default to -1 (no texture)
};
struct CircleRenderer : public Renderer {
	float radius{ 1.f };
};
struct RectRenderer : public Renderer {
	float width{ 1.f }, height{ 1.f };
};
struct AABBCollider {
	float bounding_box_size{ 1.f };
};
struct AnimalBehavior {
	std::string sound{ "moo" };
};
#endif // COMPONENT_H