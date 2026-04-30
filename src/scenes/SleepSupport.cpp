#include "scenes/SleepSupport.hpp"

#include "entities/Character.hpp"

#include <algorithm>

namespace SleepSupport
{
    SupportConfirmedSet CreateSupportConfirmedSet()
    {
        return SupportConfirmedSet{};
    }

    bool HasHorizontalSupportOverlap(const std::shared_ptr<Character> &upper,
                                     const std::shared_ptr<Character> &lower)
    {
        if (!(upper && lower))
            return false;

        const auto upPos = upper->GetPosition();
        const auto upSize = upper->GetSize();
        const auto lowPos = lower->GetPosition();
        const auto lowSize = lower->GetSize();

        const float upLeft = upPos.x - upSize.x * 0.5f;
        const float upRight = upPos.x + upSize.x * 0.5f;
        const float lowLeft = lowPos.x - lowSize.x * 0.5f;
        const float lowRight = lowPos.x + lowSize.x * 0.5f;

        const float overlap = std::min(upRight, lowRight) - std::max(upLeft, lowLeft);
        const float minOverlap = std::max(4.0f, std::min(upSize.x, lowSize.x) * 0.12f);
        return overlap >= minOverlap;
    }

    bool IsBottomSupportContact(const std::shared_ptr<Character> &sleeper,
                                const std::shared_ptr<Character> &other,
                                const glm::vec2 &contactPoint)
    {
        if (!(sleeper && other))
            return false;
        if (other->GetEntityKind() == Character::EntityKind::Slingshot)
            return false;
        if (other->GetPosition().y >= sleeper->GetPosition().y)
            return false;
        if (!HasHorizontalSupportOverlap(sleeper, other))
            return false;

        const float sleeperBottomBand = sleeper->GetPosition().y - sleeper->GetSize().y * 0.2f;
        return contactPoint.y <= sleeperBottomBand;
    }

    bool IsGeometricallySupported(const std::shared_ptr<Character> &sleeper,
                                  const std::vector<std::shared_ptr<Util::GameObject>> &elements,
                                  float worldFloorY)
    {
        if (!sleeper)
            return false;

        const auto sPos = sleeper->GetPosition();
        const auto sSize = sleeper->GetSize();
        const float sBottom = sPos.y - sSize.y * 0.5f;

        constexpr float kFloorSupportEps = 1.5f;
        if (sBottom <= worldFloorY + kFloorSupportEps)
            return true;

        constexpr float kMaxGap = 3.0f;
        constexpr float kMaxPenetration = 2.5f;
        for (const auto &element : elements)
        {
            auto other = std::dynamic_pointer_cast<Character>(element);
            if (!other || other.get() == sleeper.get())
                continue;
            if (other->GetEntityKind() == Character::EntityKind::Slingshot)
                continue;
            if (!HasHorizontalSupportOverlap(sleeper, other))
                continue;

            const auto oPos = other->GetPosition();
            const auto oSize = other->GetSize();
            const float oTop = oPos.y + oSize.y * 0.5f;
            const float gap = sBottom - oTop;

            if (gap >= -kMaxPenetration && gap <= kMaxGap && oPos.y < sPos.y)
                return true;
        }

        return false;
    }
} // namespace SleepSupport
