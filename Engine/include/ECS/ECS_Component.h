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

struct AudioData {
	struct Individual {
		unsigned resourceID;
		std::string name;

		bool loop;
		bool toPlayAtStart;
		bool locality = false;
		bool persistOnEntityDestruction = false;
		bool persistOnSceneChange = false;

		float volume = 1.0f;
		float pitch = 1.0f;
		float distanceMult = 1.0f;

		enum class Group {
			Music,
			SFX,
		} group = Group::SFX;
	};

	std::vector<Individual> audios;
	bool listener = false;
};

#endif // COMPONENT_H