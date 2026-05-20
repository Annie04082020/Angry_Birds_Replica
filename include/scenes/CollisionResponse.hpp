#ifndef SCENES_COLLISION_RESPONSE_HPP
#define SCENES_COLLISION_RESPONSE_HPP

#include <glm/glm.hpp>

struct ContactManifold;

struct DebugDrawInfo
{
    glm::vec2 pos;
    glm::vec2 scale;
    float rotation;
    glm::vec4 color;
    float ttl;
};

namespace CollisionResponse
{
    // Pre-apply accumulated impulses from the previous frame (warm-start).
    // Call once per contact after the contact list is refreshed.
    void WarmStart(ContactManifold& cm);

    // One Sequential Impulse velocity iteration.
    // Accumulates impulse with non-negativity (normal) and friction-cone (tangent) clamping.
    // damageEnabled: apply damage when impulse delta is large.
    void SolveVelocity(ContactManifold& cm, bool damageEnabled);

    // Baumgarte position correction – does NOT touch velocities.
    // Run 2-3 times after all velocity iterations per step.
    void SolvePosition(ContactManifold& cm);
}

#endif // SCENES_COLLISION_RESPONSE_HPP
