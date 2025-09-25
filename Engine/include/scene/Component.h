/*****************************************************************//**
 * \file	GameObject.h
 * \brief	This header file contains the class dleclerations
 *			For a component class 
 *
 *
 * \author	Laycock Enhui Sarah (e.laycock@digipen.edu)
 * \date	September 2025
 *********************************************************************/

#ifndef COMPONENT_H
#define COMPONENT_H
#include <string>
#include <map>

enum COMPONENT_TYPE {
	NONE=0,
	RENDERING =1,
	COLLISION =2,
	//PHYSICS=3,
	LOGIC=4
};
class GameObject;

//base component object
class Component {
protected:
	COMPONENT_TYPE type{ NONE };
	std::string c_name{ "BasicComponent" };
	GameObject* owner{ nullptr };
public:
	Component() = default;
	Component(COMPONENT_TYPE t,std::string s);
	void SetOwner(GameObject* o);
	virtual void Render();
	virtual bool Collide();
	virtual void Logic();
};


//Renderer Components
class CircleRenderer :public Component {
private:
	float radius{ 1.f };
public:
	CircleRenderer(float r);
	void Render() override;
};

class RectRenderer :public Component {
private:
	float width{ 1.f }, height{ 1.f };
public:
	RectRenderer(float w,float h);
	void Render() override;

};

class SpriteRenderer :public Component {
private:
	std::string sprite_path{};
public:
	SpriteRenderer(std::string);
	void Render() override;
};

//Collision Components
class AABBCollider :public Component {
private:
	float bounding_box_size{1.f};
public:
	AABBCollider(float bbsize);
	bool Collide() override;
};

//Logic Components
class AnimalBehaviour :public Component {
private:
	std::string sound{ "moo" };
public:
	AnimalBehaviour(std::string s);
	void Logic() override;
};

#endif
