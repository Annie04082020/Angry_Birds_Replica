#ifndef SCENES_SLEEP_SUPPORT_HPP
#define SCENES_SLEEP_SUPPORT_HPP

#include <memory>
#include <unordered_set>
#include <vector>
#include <glm/vec2.hpp>

namespace Util
{
    class GameObject;
}
class Character;

namespace SleepSupport
{
    using SupportConfirmedSet = std::unordered_set<Character *>;

    SupportConfirmedSet CreateSupportConfirmedSet();

    bool HasHorizontalSupportOverlap(const std::shared_ptr<Character> &upper,
                                     const std::shared_ptr<Character> &lower);

    bool IsBottomSupportContact(const std::shared_ptr<Character> &sleeper,
                                const std::shared_ptr<Character> &other,
                                const glm::vec2 &contactPoint);

    bool IsGeometricallySupported(const std::shared_ptr<Character> &sleeper,
                                  const std::vector<std::shared_ptr<Util::GameObject>> &elements,
                                  float worldFloorY);
}

#endif // SCENES_SLEEP_SUPPORT_HPP
