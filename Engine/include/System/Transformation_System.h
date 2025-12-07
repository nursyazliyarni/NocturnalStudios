/* !------------------------------------------------------------------------------------------------
	File: Transformation_System.h
	Project : NocturnalStudio

	This file contains ...

	 Author :
		Sarah/e.laycock(%)
		- Brief

	 Author :
		Name(%)

All content(c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
* ------------------------------------------------------------------------------------------------ */


#include "ECS/ECS_Def.h"
#include "input/Input.h"
class TransformSystem {
	
public:
	static void TransformUpdate();
	static void TransformInput(Input&,double dt);
	
};

class CollisionSystem{

public:
	
	static void CollisionUpdate(BoxCollider *coll);
private:

	static std::vector<std::pair<EntityID, EntityID>> BroadphaseCollision();
};

class PhysicsSystem{
public:
	static void PhysicsUpdate(double dt);
	static void PhysicsInput(Input&,double dt);

};
