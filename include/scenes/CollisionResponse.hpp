#ifndef SCENES_COLLISION_RESPONSE_HPP
#define SCENES_COLLISION_RESPONSE_HPP

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace Util
{
    class GameObject;
}
class Character;

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
    // Resolve collision between two characters. Returns optional debug draw entries.
    std::vector<DebugDrawInfo> ResolveCollision(const std::shared_ptr<Character> &a,
                                                const std::shared_ptr<Character> &b,
                                                const glm::vec2 &contactNormal,
                                                float penetrationDepth,
                                                const glm::vec2 &contactPoint,
                                                bool stabilizing);
}

#endif // SCENES_COLLISION_RESPONSE_HPP
