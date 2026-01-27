#include "Entity.h"
#include <algorithm>

namespace SquareCore {

Property::~Property()
{
}


void Collider::AddCollision(uint32_t entityID, int side) {
    collisions.push_back({entityID, side});
}

bool Collider::IsCollidingWith(uint32_t entityID) const {
    return std::any_of(collisions.begin(), collisions.end(),
        [entityID](const auto& collision) { return collision.first == entityID; });
}

bool Collider::HasCollisionOnSide(int side) const {
    return std::any_of(collisions.begin(), collisions.end(),
        [side](const auto& collision) { return collision.second == side; });
}

void Collider::ClearCollisions() {
    collisions.clear();
}

const std::vector<std::pair<uint32_t, int>>& Collider::GetCollisions() const {
    return collisions;
}

}