#ifndef ECS_COMPONENT_H
#define ECS_COMPONENT_H

/*!------------------------------------------------------------------------------------------------
	File: ECS_Component.h
	Project: NocturnalStudio

	This file contains the struct definition of the different component types that can be used in
	the ECS.

	Secondary Author:
		Lau Xiang Yang (30%)
		- Created the file and implemented the ECS component structs - Image (now renderer), Transform
		- Implemented AudioData


		

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include <string>
#include <type_traits>

#include "../include/math/Matrix.h"
#include "../include/math/Vector.h"

//-------------------------- Add new ECS component type here --------------------------//
struct RendererComponent;
struct Collider;

struct Transform;

struct ImageRenderer;
struct CircleRenderer;
struct RectRenderer;

struct AnimalBehavior;

struct AudioData;
struct Physics;

struct BoxCollider;

//-------------------------- ECS Component Definitions --------------------------//


//collider base struct
struct Collider {
	bool isColliding{ false };
};

//renderer base struct
struct RendererComponent {
	float opacity = 1.0f; // Default to fully opaque
	bool isVisible = true;

	Vector4D colorTint = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4D colorToAdd = {};
};


struct Transform {
	Vector2D position{ 250.f,350.f };
	Vector2D scale{100.f,100.f};
	Vector2D lastPosition = {};
	Vector2D lastScale = {};

	float rotation{};
	float height{};
	float lastRotation = rotation;
	float lastHeight = height;

	enum class CoordinateSpace {
		World,
		Viewport
	};

	CoordinateSpace space = CoordinateSpace::World;
	bool isVisible = true;
};


struct ImageRenderer: public RendererComponent {
	int textureID = -1; // Default to -1 (no texture)
};
struct CircleRenderer : public RendererComponent {
	float radius{ 1.f };
};
struct RectRenderer : public RendererComponent {
	float width{ 1.f }, height{ 1.f };
};


struct AnimalBehavior {
	std::string sound{ "moo" };
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

struct PhysicsComponent
{
	Vector2D position, velocity{ 0.f,0.f }, acceleration{ 0.f,0.f };
	bool gravityEnabled{ true };

};

struct BoxCollider :public Collider {
	float width{ 0.f }, height{ 0.f };
};
#endif // ECS_COMPONENT_H
