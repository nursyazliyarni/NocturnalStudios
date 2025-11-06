#include "ECS/ECS_Component.h"
#include "ECS/ECS.h"
#include "../include/core/Logger.h"
#include "../include/System/Transformation_System.h"

extern Logger logger;
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
