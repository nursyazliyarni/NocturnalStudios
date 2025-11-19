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


#include <rapidjson/document.h>
#include "../include/math/Matrix.h"
#include "../include/math/Vector.h"
#include "../include/math/Rand.h"

//-------------------------- Add new ECS component type here --------------------------//
struct RenderComponent;
struct Collider;
struct AudioData;
struct Transform;

struct ImageRenderer;
struct CircleRenderer;
struct RectRenderer;

struct AnimalBehavior;

struct AudioData;
struct PhysicsComponent;

struct AIComponent;

struct BoxCollider;

//-------------------------- ECS Component Definitions --------------------------//

struct BaseComponent {
	virtual ~BaseComponent() = default;

	// Each derived component should implement this
	virtual void FromJSON(const rapidjson::Value& json) = 0;
};

//collider base struct
struct Collider : public BaseComponent {
	bool isColliding{ false };
	void FromJSON(const rapidjson::Value& json);
};

//renderer base struct (renamed to avoid collision with graphics::Renderer)
struct RenderComponent : public BaseComponent {
	float opacity = 1.0f; // Default to fully opaque
	bool isVisible = true;

	Vector4D colorTint = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4D colorToAdd = {};

	void FromJSON(const rapidjson::Value& json);
};


struct Transform : public BaseComponent {
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
	bool isVisible{ true }, isRotatable{ false }, isScalable{ false }, randomPos{ false }, randomRot{ false };

	Transform():position{}, scale{}, rotation{}, height{}{
		if (randomPos)
		{
			position = { Random_Range(-100.f,100.f),Random_Range(-50.f,50.f) };
		}
		if (randomRot)
		{
			rotation = Random_Range(0.f, 6.28319f);
		}
	}
	Transform(Vector2D pos, Vector2D scl, float rot) :position{ pos }, scale{ scl }, rotation{ rot }, height{} {}
	/**
	* Returns the translated vector2d matrix
	* translates the object to x and y
	*/
	Vector2D Translate(float x, float y);

	/**
	 *  Returns the translated vector2d matrix
	 *	translates by x and y along the y axis
	 */
	Vector2D TranslateBy(float x, float y);

	/**
	 * Returns the rotated angle (in radians)
	 */
	float Rotate(float a);


	/**
	 * .
	 */
	float RotateDeg(float a);


	/**
	 * .
	 */
	Vector2D ScaleUniform(float s);

	/**
	 * .
	 */
	Vector2D Scale(float x, float y);

	void FromJSON(const rapidjson::Value& json);
};


struct ImageRenderer: public RenderComponent {
	int textureID = -1; // Default to -1 (no texture)
	std::string image_path;
	void FromJSON(const rapidjson::Value& json);

	glm::vec2 uvMin = glm::vec2(0.0f, 0.0f);
	glm::vec2 uvMax = glm::vec2(1.0f, 1.0f);
};
struct SpriteRenderer : public RenderComponent {
	std::string spritePath{};
	void FromJSON(const rapidjson::Value& json);

	glm::vec2 uvMin = glm::vec2(0.0f, 0.0f);
	glm::vec2 uvMax = glm::vec2(1.0f, 1.0f);
};
struct CircleRenderer : public RenderComponent {
	float radius{ 1.f };
	void FromJSON(const rapidjson::Value& json);
};
struct RectRenderer : public RenderComponent {
	float width{ 1.f }, height{ 1.f };
	void FromJSON(const rapidjson::Value& json);
};


struct AnimalBehavior {
	double health_bar, hunger_bar, sanity_bar;
	void FromJSON(const rapidjson::Value& json);
};

struct AudioData : public BaseComponent {
	struct IndividualAudioData {
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
			RESERVED1,
			RESERVED2
		} group = Group::SFX;
	};

	std::vector<IndividualAudioData> audios;
	bool listener = false;

	void FromJSON(const rapidjson::Value& json);
};

struct PhysicsComponent : public BaseComponent
{
	Vector2D position, velocity{ 0.f,0.f }, acceleration{ 0.f,0.f };
	bool gravityEnabled{ true };
	void FromJSON(const rapidjson::Value& json);
};


struct AIComponent : public BaseComponent {
	enum class State { Wander = 0, Chase = 1, Flee = 2 };

	State  state = State::Wander;
	float  maxSpeed = 0.28f;  // normalized units per second (0..1 space)
	float  accel = 4.0f;   // max change in speed per second
	float  wanderJitter = 3.2f;   // angular noise strength (rad/s)
	float  chaseStrength = 1.0f;   // weight toward attractor
	float  fleeStrength = 1.2f;   // weight away from repulsor
	float  avoidRadius = 0.08f;  // separation radius vs neighbors

	// internal memory for wander heading (radians)
	float  wanderAngle = 0.0f;

	std::string targetName;   // optional: entity name to chase/flee
	float targetWeight = 1.0f; // how strong target influence is
	void FromJSON(const rapidjson::Value& json);

};



struct BoxCollider :public Collider {
	float width{ 0.f }, height{ 0.f };
	void FromJSON(const rapidjson::Value& json);
};

struct AABBCollider :public BoxCollider {
	void FromJSON(const rapidjson::Value& json);
};
#endif // ECS_COMPONENT_H
