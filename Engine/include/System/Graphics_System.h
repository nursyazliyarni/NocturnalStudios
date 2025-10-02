// Should Inherit from system class
// Graphics system should swap FBOs to perform offscreen rendering by calling render pipeline functions of the renderer
// before swapping back to the default framebuffer to be drawn on a ImGUI window

// i.e
//class RenderSystem : public System {
//public:
//    RenderSystem(ECS& ecs, Renderer& renderer)
//        : System(ecs, "RenderSystem"), renderer(renderer) {
//    }
//
//    void update(double /*deltaTime*/) final {
//        for (EntityID id : ECSRegistry.getAllEntities()) {
//            auto* t = ECSRegistry.getComponent<Transform>(id);
//            auto* img = ECSRegistry.getComponent<Image>(id);
//            if (t && img && img->isVisible && t->isVisible) {
//                renderer.texturedTri(
//                    img->textureID,
//                    t->position.x(), t->position.y(),
//                    t->scale.x(), t->scale.y()
//                );
//            }
//        }
//    }
//private:
//    Renderer& renderer;
//};