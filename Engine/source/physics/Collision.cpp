// collision checks & responses

#include "physics/Collision.h"

bool checkCollision(float x1, float y1, float w1, float h1,	float x2, float y2, float w2, float h2) {
	//collision x-axis
	bool collisionX = x1 + w1 >= x2 && x2 + w2 >= x1;
	//collision y-axis
	bool collisionY = y1 + h1 >= y2 && y2 + h2 >= y1;
	//collision only if on both axes
	return collisionX && collisionY;
}

