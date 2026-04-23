#ifndef SCENES_COLLISION_UTILS_HPP
#define SCENES_COLLISION_UTILS_HPP

#include "Character.hpp"
#include <glm/vec2.hpp>

namespace CollisionUtils
{
    float GetRestitution(Character::MaterialType mat);
    float GetFriction(Character::MaterialType mat);

    // 2D cross product (returns scalar)
    inline float Cross(const glm::vec2 &a, const glm::vec2 &b)
    {
        return a.x * b.y - a.y * b.x;
    }

    // Compute MTV (minimum translation vector) between two OBBs using SAT.
    // Returns true if overlap (collision). Outputs a unit normal pointing from A to B,
    // penetration depth (positive), and an approximate contact point.
    bool ComputeOBBMTV(const Character &A, const Character &B,
                       glm::vec2 &outNormal, float &outDepth,
                       glm::vec2 &outContactPoint);
}

#endif // SCENES_COLLISION_UTILS_HPP
