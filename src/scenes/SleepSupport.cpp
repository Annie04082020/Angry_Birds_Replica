#include "scenes/SleepSupport.hpp"

#include "entities/Character.hpp"

#include <algorithm>
#include <cmath>

namespace SleepSupport
{
    namespace
    {
        glm::vec2 GetSupportBoundsSize(const std::shared_ptr<Character> &character)
        {
            if (!character)
                return {0.0f, 0.0f};

            const glm::vec2 size = character->GetSize();
            const float halfW = size.x * 0.5f;
            const float halfH = size.y * 0.5f;
            const float rotation = character->GetTransform().rotation;
            const float absCos = std::fabs(std::cos(rotation));
            const float absSin = std::fabs(std::sin(rotation));

            const float aabbHalfW = halfW * absCos + halfH * absSin;
            const float aabbHalfH = halfW * absSin + halfH * absCos;
            return {aabbHalfW * 2.0f, aabbHalfH * 2.0f};
        }
    } // namespace

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
        const auto upSize = GetSupportBoundsSize(upper);
        const auto lowPos = lower->GetPosition();
        const auto lowSize = GetSupportBoundsSize(lower);

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

        const float sleeperBottomBand = sleeper->GetPosition().y - GetSupportBoundsSize(sleeper).y * 0.2f;
        return contactPoint.y <= sleeperBottomBand;
    }

    bool IsGeometricallySupported(const std::shared_ptr<Character> &sleeper,
                                  const std::vector<std::shared_ptr<Util::GameObject>> &elements,
                                  float worldFloorY)
    {
        if (!sleeper)
            return false;

        const auto sPos = sleeper->GetPosition();
        const auto sSize = GetSupportBoundsSize(sleeper);
        const float sBottom = sPos.y - sSize.y * 0.5f;

        constexpr float kFloorSupportEps = 1.5f;
        if (sBottom <= worldFloorY + kFloorSupportEps)
            return true;

        // Allow larger gaps/penetrations so objects that are just barely out of
        // perfect alignment after StabilizeEnvironment are still treated as
        // "supported" and are not spuriously woken up (which would cause them to
        // free-fall a few pixels and produce damaging collision impulses).
        constexpr float kMaxGap = 6.0f;
        constexpr float kMaxPenetration = 5.0f;
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
            const auto oSize = GetSupportBoundsSize(other);
            const float oTop = oPos.y + oSize.y * 0.5f;
            const float gap = sBottom - oTop;

            if (gap >= -kMaxPenetration && gap <= kMaxGap && oPos.y < sPos.y)
                return true;
        }

        return false;
    }
} // namespace SleepSupport
