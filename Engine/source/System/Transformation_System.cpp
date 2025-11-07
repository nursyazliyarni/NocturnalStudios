#include "ECS/ECS_Component.h"
#include "ECS/ECS.h"
#include "../include/core/Logger.h"
#include "../include/System/Transformation_System.h"

extern Logger logger;


namespace {
	ECS& ecs = ECS::getECS();
	EntityID selectedID = NO_ENTITY_SELECTED;
}
/**
	* Returns the translated vector2d matrix
	* translates the object to x and y
	*/
Vector2D Transform::Translate(float x, float y) {
	position = { x, y };
	return position;
}

/**
 *  Returns the translated vector2d matrix
 *	translates by x and y along the y axis
 */
Vector2D Transform::TranslateBy(float x, float y) {
	position += Vector2D{x, y};
	return position;
}

/**
 * Returns the rotated angle (in radians)
 */
float Transform::Rotate(float a) {
	rotation += a;
	return rotation;
}


/**
 * .
 */
float Transform::RotateDeg(float a) {
	rotation += a * (static_cast<float>(PI) / 180.f);
	return rotation;
}



/**
 * .
 */
Vector2D Transform::ScaleUniform(float s) {
	scale *= s;
	return scale;
}

/**
 * .
 */
Vector2D Transform::Scale(float x, float y) {
	scale ={ scale.x()*x,scale.y()*y };
	return scale;
}


void TransformSystem::TransformUpdate() {
	ECS& ecs = ECS::getECS();
	std::vector<EntityID> ectities = ecs.getAllEntities();
	for (EntityID e:ectities)
	{
		Transform* t = ecs.getComponent<Transform>(e);
		if (t)
		{
			t->lastPosition = t->position;
			t->lastHeight = t->height;
			t->lastScale = t->scale;
			t->lastRotation = t->rotation;
			std::string poob{ "Transform updated for entity " };
			poob.append(std::to_string(e));
			logger.log(poob);
		}
	}
}

void TransformSystem::TransformInput(Input& in,double dt) {
	ECS& ecs = ECS::getECS();

	if (selectedID == NO_ENTITY_SELECTED)
	{
		if (in.WasPressed(GLFW_KEY_SPACE)) //up
		{
			selectedID = ecs.getAllEntities().front();
		}
	}

	Transform* t = ecs.getComponent<Transform>(selectedID);

	

	if (t)
	{
		Vector2D movedir{};
		if (in.IsDown(GLFW_KEY_W)) //up
		{
			movedir.y() -= static_cast<float>(dt * 1.0);
		}
		else if (in.IsDown(GLFW_KEY_S))
		{
			movedir.y() += static_cast<float>(dt * 1.0);
		}
		if (in.IsDown(GLFW_KEY_A)) //up
		{
			movedir.x() -= static_cast<float>(dt * 1.0);
		}
		else if (in.IsDown(GLFW_KEY_D))
		{
			movedir.x() += static_cast<float>(dt * 1.0);
		}

		float rot{t->rotation};
		if (in.IsDown(GLFW_KEY_K))
		{
			rot -= static_cast<float>(dt * 100.0);
		}
		else if (in.IsDown(GLFW_KEY_J)) {
			rot += static_cast<float>(dt * 100.0);
		}

		float scaling{1.f};
		if (in.IsDown(GLFW_KEY_U))
		{
			scaling -= static_cast<float>(dt * 2.0);
		}
		else if (in.IsDown(GLFW_KEY_I)) {
			scaling += static_cast<float>(dt * 2.0);
		}


		t->ScaleUniform(scaling);
		if (AABBCollider *collider= ecs.getComponent<AABBCollider>(selectedID))
		{
			collider->height *= scaling;
			collider->width *= scaling;
		}
		t->rotation = rot;
		t->TranslateBy(movedir.x(),movedir.y());
	}
	
	
	
}

void CollisionSystem::CollisionUpdate(){

}

void PhysicsSystem::PhysicsUpdate() {

}

void PhysicsSystem::PhysicsInput(Input& in,double dt) {
	if (selectedID == NO_ENTITY_SELECTED)
	{
		if (in.WasPressed(GLFW_KEY_SPACE)) //up
		{
			selectedID = ecs.getAllEntities().front();
		}
	}
}
