/*****************************************************************//**
 * \file	GameObject.cpp
 * \brief	This source file contains the definitions for
 *			the Engine's GameObjects Class
 *
 *
 * \author	Laycock Enhui Sarah (e.laycock@digipen.edu)
 * \date	September 2025
 *********************************************************************/


#include "./scene/GameObject.h"
#include"./scene/Factory.h"

// [====================*====================]
//  Constructors & Destructors
// [====================*====================]
// Default Constructor

GameObject::GameObject(std::string n) :id{}, name{ name }, active{ false }, enabled{ false }, components{}, parent_object{} {}

GameObject::~GameObject() { ClearParent(); }


// [====================*====================]
//  Lifecycle Functions
// [====================*====================]
void GameObject::Start(){}

void GameObject::Update(){}

void GameObject::Render(){}

void GameObject::Kill(){}


// [====================*====================]
//  Transform Handling Functions
// [====================*====================]
void GameObject::Pos(const Vector3D& pos) { global_position=pos; }
Vector3D GameObject::Pos() const { return global_position; }

//void GameObject::Rot(const Matrix<4>& rot){}
//Matrix<4> GameObject::Rot() const{}

//void GameObject::Scale(const Vector3D& scale){}
//Vector3D GameObject::Scale() const{}

Matrix<4> GameObject::Transform() const { return global_transform; }


// [====================*====================]
//  Getters and Setters
// [====================*====================]
size_t GameObject::ID()const { return num_ID; }

void GameObject::Activate() { active = true; enabled = true; }

bool GameObject::ActiveStatus() const { return active; }
void GameObject::ActiveStatus(bool state) { active = state; }

bool GameObject::EnabledStatus() const { return enabled; }
void GameObject::EnabledStatus(bool state) { enabled = state; }


// [====================*====================]
//  Parenting
// [====================*====================]
GameObject* GameObject::Parent() const { return parent_object; }
bool GameObject::MakeParent(GameObject* to_parent){
	if (to_parent&&to_parent->ActiveStatus())
	{
		ClearParent();
		parent_object = to_parent;
		return true;
	}
	else 
	{
		return false;
	}
}
void GameObject::ClearParent() {parent_object = nullptr;}

// [====================*====================]
//  Print & Load
// [====================*====================]
void GameObject::Print(std::ostream& out) const {
	out << "GameObject " << num_ID
		<< " - Active: " << active << ", Enabled: " << enabled << '\n'
		<< "Position: " << global_position << '\n';
}

void GameObject::Load(std::istream&) {

}

std::istream& operator>>(std::istream& in, GameObject& obj) {
	return in;
}

std::ostream& operator<<( std::ostream& out,const GameObject& obj) {
	obj.Print(out);
	return out;
}

namespace GameObjects {
	void GameObject_test() {

	}
}