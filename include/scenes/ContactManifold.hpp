#pragma once
#include <glm/glm.hpp>

class Character;

// Persists contact state across frames for Sequential Impulse warm-starting.
// The solver accumulates impulses (clamped to valid range) and re-applies them
// as the initial guess next frame, so it starts from near-correct values.
struct ContactManifold
{
    Character* a = nullptr;
    Character* b = nullptr;

    glm::vec2 normal;        // collision normal, pointing from a toward b
    glm::vec2 tangent;       // always perp(normal) = vec2(-n.y, n.x)
    glm::vec2 contactPoint;

    float penetration = 0.f;

    // Accumulated impulses – carried across frames for warm-starting.
    float normalImpulse  = 0.f; // >= 0  (can only push, never pull)
    float tangentImpulse = 0.f; // clamped to [-mu*jn, mu*jn]
    // Accumulate normal impulse over the solver iterations for the current
    // physics step so damage can be applied once per step instead of once
    // per solver iteration (which would multiply damage unexpectedly).
    float frameAccumulatedNormalImpulse = 0.f;

    bool active = false;

    bool Matches(const Character* x, const Character* y) const noexcept
    {
        return (a == x && b == y) || (a == y && b == x);
    }

    bool SameOrder(const Character* x, const Character* y) const noexcept
    {
        return a == x && b == y;
    }
};
