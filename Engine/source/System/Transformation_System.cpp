#include "ECS/ECS_Component.h"
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


//float Transform::RotateDeg(float a) {
//	rotation += a * ((float)PI / 180.f);
//	return a;
//}


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
