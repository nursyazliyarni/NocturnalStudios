// bounding box collision
#include "physics/AABB.h"

bool checkAABBCollision(const Vector2D& pos1, const Vector2D& size1, const Vector2D& pos2, const Vector2D& size2) {
    return (pos1.x() + size1.x() >= pos2.x() && pos2.x() + size2.x() >= pos1.x() &&
        pos1.y() + size1.y() >= pos2.y() && pos2.y() + size2.y() >= pos1.y());
}