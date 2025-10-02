// Should update the physics (if any), position, forces and bounding boxes of all
// entities (game objects) every frame if the game is running
// Should skip the frame if the game is paused.

// i.e
// class PhysicsSystem : public System {
// public:
//     PhysicsSystem(ECS& ecs) : System(ecs, "PhysicsSystem") {}
//
//     void update(double deltaTime) override {
//         for (EntityID id : ECSRegistry.getAllEntities()) {
//             auto* transform = ECSRegistry.getComponent<Transform>(id);
//             auto* rigidbody = ECSRegistry.getComponent<Rigidbody>(id);
//             if (transform && rigidbody) {
//                 // Update physics
//             }
//         }
//     }
// };