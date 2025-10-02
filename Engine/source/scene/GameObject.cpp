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

GameObject::GameObject(std::string n) :id{}, name{ n }, active{ false }, enabled{ false }, components{}, parent_object{} {}

GameObject::GameObject(const GameObject& obj) :id{}, name{ obj.name }, active{ obj.active }, enabled{ obj.enabled }, components{}, parent_object{ obj.parent_object } {
	for ( const auto& comp:obj.components)
	{
		components.push_back(comp->Clone());
	}
	for (const auto& comp : components)
	{
		comp->SetOwner(this);
	}
}


GameObject::~GameObject() {
	ClearParent(); //std::cout << "destroyed " << name << '\n';
}


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
void GameObject::Attach(std::unique_ptr<Component>&& comp) {
	comp->SetOwner(this);
	components.push_back(std::move(comp));
}

// [====================*====================]
//  Getters and Setters
// [====================*====================]
size_t GameObject::ID()const { return id; }

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
	out << "GameObject " << name
		<< " - Active: " << active << ", Enabled: " << enabled<<", Components: ";
	for (const auto& comp : components)
	{
		out << comp->ComponenetName() << ' ';
	}
	out << '\n';
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