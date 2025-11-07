#include "ECS/ECS_Def.h"
#include "input/Input.h"
class TransformSystem {
	
public:
	
	static void TransformUpdate();
	static void TransformInput(Input&,double dt);
	
};

class CollisionSystem{

public:
	static void CollisionUpdate();
};

class PhysicsSystem{
public:

	static void PhysicsUpdate();
	static void PhysicsInput(Input&,double dt);

};
