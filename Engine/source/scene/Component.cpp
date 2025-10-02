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

CircleRenderer::CircleRenderer(float r) :Component{ RENDERING,"Cricle Renderer" }, radius{ r } {}
CircleRenderer::CircleRenderer(const rapidjson::Value& j) :Component{ RENDERING,"Cricle Renderer" }, radius{} {
	if (j["radius"].IsFloat())
	{
		radius = j["radius"].GetFloat();
		//std::cout<<" radius: "<< j["radius"].GetFloat();
	}
}
CircleRenderer::CircleRenderer(const CircleRenderer& c) :Component{ c }, radius{ c.radius } {}

std::unique_ptr<Component> CircleRenderer::Clone() const {
	return std::make_unique<CircleRenderer>(CircleRenderer(*this));
}

RectRenderer::RectRenderer(float w, float h) :Component{ RENDERING,"Rect Renderer" }, width{ w }, height{ h } {}
RectRenderer::RectRenderer(const rapidjson::Value& j) :Component{ RENDERING,"Rect Renderer" }, width{  }, height{  } {
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
RectRenderer::RectRenderer(const RectRenderer& c) :Component{ c }, width{ c.width }, height{ c.height } {}
std::unique_ptr<Component> RectRenderer::Clone() const {
	return std::make_unique<RectRenderer>(RectRenderer(*this));
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

AnimalBehaviour::AnimalBehaviour(std::string s):Component{ LOGIC,"Animal Behaviour" },sound{s}{}
AnimalBehaviour::AnimalBehaviour(const rapidjson::Value& j) :Component{ LOGIC,"Animal Behaviour" }, sound{  } {
	if (j["sound"].IsString())
	{
		sound = j["sound"].GetString();
	}
}
AnimalBehaviour::AnimalBehaviour(const AnimalBehaviour& c) :Component{ c }, sound{ c.sound } {}
std::unique_ptr<Component> AnimalBehaviour::Clone() const {
	return std::make_unique<AnimalBehaviour>(AnimalBehaviour(*this));
}

void Component::Render(Renderer&) {
	
}
bool  Component::Collide() {
	return true;
}
void  Component::Logic() {

}



void CircleRenderer::Render(Renderer&) {

}
void RectRenderer::Render(Renderer&) {

}
void SpriteRenderer::Render(Renderer& r) {
	r.get_sprite_vector().push_back({ 4, Random_Range(-0.3f,0.3f), Random_Range(-0.3f,0.3f), Random_Range(0.25f,0.55f), Random_Range(0.10f,0.4f), 0.0f});
}
bool AABBCollider::Collide() {
	return true;
}
void AnimalBehaviour::Logic() {
	
}