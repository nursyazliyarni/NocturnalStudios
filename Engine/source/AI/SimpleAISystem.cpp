//#include "AI/SimpleAISystem.h"
//#include <cmath>
//#include <algorithm>
//#include "ECS/ECS.h"
//#include "ECS/ECS_Component.h"
//
//namespace {
//    constexpr EntityID MAX_SCAN = 5000;
//
//    inline void limit(float& x, float& y, float m) {
//        float l2 = x * x + y * y;
//        if (l2 > m * m) {
//            float inv = 1.0f / std::sqrt(l2);
//            x *= inv * m; y *= inv * m;
//        }
//    }
//    inline void steerTo(float tx, float ty, float px, float py, float speed, float k, float& sx, float& sy) {
//        float dx = tx - px, dy = ty - py; float l = std::sqrt(dx * dx + dy * dy); if (l < 1e-5f) return; dx /= l; dy /= l;
//        sx += dx * speed * k; sy += dy * speed * k;
//    }
//    inline void steerAway(float tx, float ty, float px, float py, float speed, float k, float& sx, float& sy) {
//        float dx = px - tx, dy = py - ty; float l = std::sqrt(dx * dx + dy * dy); if (l < 1e-5f) return; dx /= l; dy /= l;
//        sx += dx * speed * k; sy += dy * speed * k;
//    }
//    inline void avoidRect(float px, float py, float pr, float rx, float ry, float rw, float rh, float k, float& sx, float& sy) {
//        float cx = std::clamp(px, rx, rx + rw), cy = std::clamp(py, ry, ry + rh);
//        float dx = px - cx, dy = py - cy; float d2 = dx * dx + dy * dy; float r = std::max(pr, 1e-4f);
//        if (d2 < r * r) {
//            float l = std::sqrt(std::max(d2, 1e-6f));
//            dx = (l > 0 ? dx / l : 0); dy = (l > 0 ? dy / l : 0);
//            sx += dx * k; sy += dy * k;
//        }
//    }
//}
//
//void SimpleAISystem::Update(ECS& ecs, double dt, const std::string& sceneName, float cursorX, float cursorY, bool cursorInside)
//{
//    float fdt = (dt > 0.0 ? (float)dt : 1.f / 60.f);
//
//    for (EntityID e = 0; e < MAX_SCAN; ++e) {
//        auto* ai = ecs.getComponent<SimpleAI>(e);
//        if (!ai) continue;
//
//        auto* t = ecs.getComponent<Transform>(e);
//        if (!t) continue;
//
//        auto* tag = ecs.getComponent<SceneTag>(e);
//        if (!tag || tag->scene != sceneName) continue;
//
//
//        auto* pc = ecs.getComponent<PhysicsComponent>(e);
//        float vx = (pc ? pc->velocity.x() : 0.f);
//        float vy = (pc ? pc->velocity.y() : 0.f);
//
//        // steering accumulator
//        float sx = 0.f, sy = 0.f;
//
//        // wander baseline
//        ai->wanderAngle += ai->wanderJitter * fdt * ((std::rand() / (float)RAND_MAX) - 0.5f);
//        sx += std::cos(ai->wanderAngle) * (ai->maxSpeed * 0.35f);
//        sy += std::sin(ai->wanderAngle) * (ai->maxSpeed * 0.35f);
//
//        // state-based steering
//        if (cursorInside) {
//            if (ai->state == AgentState::ChaseCursor)
//                steerTo(cursorX, cursorY, t->position.x(), t->position.y(), ai->maxSpeed, ai->chaseStrength, sx, sy);
//            else if (ai->state == AgentState::FleeFromCursor)
//                steerAway(cursorX, cursorY, t->position.x(), t->position.y(), ai->maxSpeed, ai->fleeStrength, sx, sy);
//        }
//
//        // simple avoidance against big rects / colliders (owl, food, meds, toy…)
//        for (EntityID o = 0; o < MAX_SCAN; ++o) {
//            if (o == e) continue;
//            auto* bt = ecs.getComponent<Transform>(o);
//            if (!bt) continue;
//            float rw = bt->scale.x(), rh = bt->scale.y();
//            if (ecs.getComponent<BoxCollider>(o) || (rw * rh > 0.03f)) {
//                avoidRect(t->position.x(), t->position.y(), ai->avoidRadius,
//                    bt->position.x(), bt->position.y(), rw, rh, 0.8f, sx, sy);
//            }
//        }
//
//        // integrate velocity/position
//        vx += sx * ai->accel * fdt; vy += sy * ai->accel * fdt; limit(vx, vy, ai->maxSpeed);
//        t->position.x() += vx * fdt; t->position.y() += vy * fdt;
//
//        // keep inside [0..1] visible area considering size
//        float minX = 0.f, minY = 0.f, maxX = 1.f - t->scale.x(), maxY = 1.f - t->scale.y();
//        t->position.x() = std::clamp(t->position.x(), minX, maxX);
//        t->position.y() = std::clamp(t->position.y(), minY, maxY);
//
//        if (pc) { pc->velocity.x() = vx; pc->velocity.y() = vy; }
//    }
//}
//
//bool SimpleAISystem::IsHit(ECS& ecs, EntityID e, float nx, float ny) const {
//    auto* ai = ecs.getComponent<SimpleAI>(e); if (!ai) return false;
//    auto* t = ecs.getComponent<Transform>(e); if (!t)  return false;
//    float x = t->position.x(), y = t->position.y(), w = t->scale.x(), h = t->scale.y();
//    return (nx >= x && nx <= x + w && ny >= y && ny <= y + h);
//}
//
//bool SimpleAISystem::ToggleStateOnHit(ECS& ecs, float nx, float ny, const std::string& sceneName)
//{
//    for (EntityID e = 0; e < MAX_SCAN; ++e) {
//        auto* ai = ecs.getComponent<SimpleAI>(e); if (!ai) continue;
//
//        auto* tag = ecs.getComponent<SceneTag>(e);
//        if (!tag || tag->scene != sceneName) continue;
//
//
//        if (!IsHit(ecs, e, nx, ny)) continue;
//
//        if (ai->state == AgentState::Wander)         ai->state = AgentState::ChaseCursor;
//        else if (ai->state == AgentState::ChaseCursor)    ai->state = AgentState::FleeFromCursor;
//        else if (ai->state == AgentState::FleeFromCursor) ai->state = AgentState::ChaseCursor;
//
//        ai->selected = true;
//        return true; // we hit an AI entity
//    }
//    return false;
//}
//
//void SimpleAISystem::SetAll(ECS& ecs, int s, const std::string& sceneName)
//{
//    AgentState st = AgentState::Wander;
//    if (s == 1) st = AgentState::ChaseCursor;
//    else if (s == 2) st = AgentState::FleeFromCursor;
//
//    for (EntityID e = 0; e < MAX_SCAN; ++e) {
//        auto* tag = ecs.getComponent<SceneTag>(e);
//        if (!tag || tag->scene != sceneName) continue;
//
//        auto* ai = ecs.getComponent<SimpleAI>(e); if (!ai) continue;
//        ai->state = st; ai->selected = false;
//    }
//}
