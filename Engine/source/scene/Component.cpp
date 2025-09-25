#include "./scene/Component.h"


Component::Component(COMPONENT_TYPE t, std::string s) :type{ t },c_name { s }, owner{ nullptr } {}

void Component::SetOwner(GameObject* o) { owner = o; }

CircleRenderer::CircleRenderer(float r) :Component{ RENDERING,"Cricle Renderer" }, radius{ r } {}

RectRenderer::RectRenderer(float w, float h) :Component{ RENDERING,"Rect Renderer" }, width{ w }, height{ h } {}

SpriteRenderer::SpriteRenderer(std::string path) :Component{ RENDERING,"Sprite Renderer" }, sprite_path{ path } {}

AABBCollider::AABBCollider(float bbsize):Component{ COLLISION,"AABB Collider" },bounding_box_size{bbsize}{}

AnimalBehaviour::AnimalBehaviour(std::string s):Component{ LOGIC,"Animal Behaviour" },sound{s}{}

void CircleRenderer::Render() {

}
void RectRenderer::Render() {

}
void SpriteRenderer::Render() {

}
bool AABBCollider::Collide() {
	return true;
}
void AnimalBehaviour::Logic() {

}