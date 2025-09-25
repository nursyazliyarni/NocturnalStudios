#include "./scene/Component.h"


Component::Component(std::string name) :c_name{ name }, owner{ nullptr } {}\
void Component::SetOwner(GameObject* o) { owner = o; }
CircleRenderer::CircleRenderer(float r) :Component{ "Cricle Renderer" }, radius{ r } {}


void CircleRenderer::Render() {

}
void RectRenderer::Render() {

}
void SpriteRenderer::Render() {

}