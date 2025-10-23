/* !------------------------------------------------------------------------------------------------
	File: Component.cpp
	Project : NocturnalStudio

	This file contains ...

	Primary Author :
		Name(%)
		- Brief

	Secondary Author :
		Name(%)
		- Brief

All content(c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
* ------------------------------------------------------------------------------------------------ */


#include "./scene/Component.h"
#include "./math/Rand.h"
#include <iostream>

COMPONENT_TYPE Component::Type() const { return type; }

Component::Component(COMPONENT_TYPE t, std::string s) :type{ t }, c_name{ s }, owner{ nullptr } {}
Component::Component(const Component& comp) :type{ comp.type }, c_name{ comp.c_name }, owner{ nullptr } {}
std::string Component::ComponenetName() const {
	return c_name;
}
void Component::SetOwner(GameObject* o) { owner = o; }
void Component::Transform(){}
CircRenderer::CircRenderer(float r) :Component{ RENDERING,"Cricle Renderer" }, radius{ r } {}
CircRenderer::CircRenderer(const rapidjson::Value& j) :Component{ RENDERING,"Cricle Renderer" }, radius{} {
	if (j["radius"].IsFloat())
	{
		radius = j["radius"].GetFloat();
		//std::cout<<" radius: "<< j["radius"].GetFloat();
	}
}
CircRenderer::CircRenderer(const CircRenderer& c) :Component{ c }, radius{ c.radius } {}

std::unique_ptr<Component> CircRenderer::Clone() const {
	return std::make_unique<CircRenderer>(CircRenderer(*this));
}

RectaRenderer::RectaRenderer(float w, float h) :Component{ RENDERING,"Rect Renderer" }, width{ w }, height{ h } {}
RectaRenderer::RectaRenderer(const rapidjson::Value& j) :Component{ RENDERING,"Rect Renderer" }, width{  }, height{  } {
	if (j["width"].IsFloat())
	{
		width = j["width"].GetFloat();
		//std::cout << " width: " << j["width"].GetFloat();
	}
	if (j["height"].IsFloat())
	{
		height = j["height"].GetFloat();
		//std::cout << " height: " << j["height"].GetFloat();
	}
}
RectaRenderer::RectaRenderer(const RectaRenderer& c) :Component{ c }, width{ c.width }, height{ c.height } {}
std::unique_ptr<Component> RectaRenderer::Clone() const {
	return std::make_unique<RectaRenderer>(RectaRenderer(*this));
}


SpriteRenderer::SpriteRenderer(std::string path) :Component{ RENDERING,"Sprite Renderer" }, sprite_path{ path } {}
SpriteRenderer::SpriteRenderer(const rapidjson::Value& j) :Component{ RENDERING,"Sprite Renderer" }, sprite_path{ } {
	if (j["sprite_path"].IsString())
	{
		sprite_path = j["sprite_path"].GetString();
	}
}
SpriteRenderer::SpriteRenderer(const SpriteRenderer& c) :Component{ c }, sprite_path{ c.sprite_path} {}
std::unique_ptr<Component> SpriteRenderer::Clone() const {
	return std::make_unique<SpriteRenderer>(SpriteRenderer(*this));
}

AABBCollider::AABBCollider(float bbsize):Component{ COLLISION,"AABB Collider" },bounding_box_size{bbsize}{}
AABBCollider::AABBCollider(const rapidjson::Value& j) :Component{ COLLISION,"AABB Collider" }, bounding_box_size{  } {
	if (j["bounding_box_size"].IsFloat())
	{
		bounding_box_size = j["bounding_box_size"].GetFloat();
	}
}
AABBCollider::AABBCollider(const AABBCollider& c) :Component{ c }, bounding_box_size{ c.bounding_box_size }{}
std::unique_ptr<Component> AABBCollider::Clone() const {
	return std::make_unique<AABBCollider>(AABBCollider(*this));
}

AnimalBehaviou::AnimalBehaviou(std::string s):Component{ LOGIC,"Animal Behaviour" },sound{s}{}
AnimalBehaviou::AnimalBehaviou(const rapidjson::Value& j) :Component{ LOGIC,"Animal Behaviour" }, sound{  } {
	if (j["sound"].IsString())
	{
		sound = j["sound"].GetString();
	}
}
AnimalBehaviou::AnimalBehaviou(const AnimalBehaviou& c) :Component{ c }, sound{ c.sound } {}
std::unique_ptr<Component> AnimalBehaviou::Clone() const {
	return std::make_unique<AnimalBehaviou>(AnimalBehaviou(*this));
}

void Component::Render(Renderer&) {
	
}
bool  Component::Collide() {
	return true;
}
void  Component::Logic() {

}



void CircRenderer::Render(Renderer&) {

}
void RectaRenderer::Render(Renderer&) {

}
//void SpriteRenderer::Render(Renderer& r) {
//	r.get_sprite_vector().push_back({ 5, Random_Range(0.f,0.75f), Random_Range(0.f,0.75f),0.15, 0.15, Random_Range(0.0f,360.f)});
//}
void SpriteRenderer::Render(Renderer& r) {
	// TODO: In LevelManager or GameObject spawn, push the sprite with prefab data.
	// This Render() should only ensure the sprite is drawn later, not spawned here.
}


bool AABBCollider::Collide() {
	return true;
}
void AnimalBehaviou::Logic() {
	
}