/*****************************************************************//**
 * \file	GameObject.h
 * \brief	This header file contains the declarations for
 *			the Engine's GameObjects Class
 * 
 * 
 * \author	Laycock Enhui Sarah (e.laycock@digipen.edu)
 * \date	September 2025
 *********************************************************************/

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <iostream>
#include "./math/Matrix.h"


// [========================================*========================================]
//  Base GameObject Class
// [========================================*========================================]
class GameObject {
private:
	size_t num_ID;
	bool active, enabled;
	Vector3D global_position, local_position;
	Matrix<4> global_transform;
	GameObject* parent_object;
public:

	// [====================*====================]
	//  Constructors & Destructors
	// [====================*====================]
	GameObject();
	GameObject(GameObject const& to_copy );
	GameObject& operator=(GameObject const& other);


	~GameObject();


	// [====================*====================]
	//  Lifecycle Functions
	// [====================*====================]
	virtual void Start();

	virtual void Update();

	virtual void Render();

	virtual void Kill();


	// [====================*====================]
	//  Transform Handling Functions
	// [====================*====================]
	void Pos(const Vector3D& pos);
	Vector3D Pos() const;

	//void Rot(const Matrix<4>& rot);
	//Matrix<4> Rot() const;

	//void Scale(const Vector3D& scale);
	//Vector3D Scale() const;

	Matrix<4> Transform() const;


	// [====================*====================]
	//  Getters and Setters
	// [====================*====================]
	size_t ID()const;

	void Activate();

	bool ActiveStatus() const;
	void ActiveStatus(bool);

	bool EnabledStatus() const;
	void EnabledStatus(bool);


	// [====================*====================]
	//  Parenting
	// [====================*====================]
	GameObject* Parent() const;
	bool MakeParent(GameObject*);
	void ClearParent();
	//do i add children?

	// [====================*====================]
	//  Print
	// [====================*====================]
	virtual void Print(std::ostream&) const;
};
std::ostream& operator<<(std::ostream& out, const GameObject& obj);
namespace GameObjects{ 
	void GameObject_test(); 
}


#endif
