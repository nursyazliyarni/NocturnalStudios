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
class GameObject;

//base component object
class Component {
protected:
	std::string c_name{ "Basic Component" };
	GameObject* owner{ nullptr };
public:
	Component() = default;
	Component(std::string);
	void SetOwner(GameObject* o);
	virtual void Render();
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
	void Render() override;

};

class SpriteRenderer :public Component {
private:
	std::string sprite_path{};
public:
	void Render() override;
};

#endif
