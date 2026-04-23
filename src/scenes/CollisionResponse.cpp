#include "scenes/CollisionResponse.hpp"

#include "CollisionUtils.hpp"
#include "../entities/Character.hpp"
#include "scenes/DebugUtils.hpp"

#include <glm/glm.hpp>
#include <cfloat>
#include <cmath>

using namespace CollisionResponse;

std::vector<DebugDrawInfo> CollisionResponse::ResolveCollision(const std::shared_ptr<Character> &a,
                                                               const std::shared_ptr<Character> &b,
                                                               const glm::vec2 &contactNormal,
                                                               float penetrationDepth,
                                                               const glm::vec2 &contactPoint,
                                                               bool stabilizing)
{
    std::vector<DebugDrawInfo> debugOut;

    auto ca = a;
    auto cb = b;
    if (!(ca && cb))
        return debugOut;

    // Slingshot should be visual only
    if (ca->GetEntityKind() == Character::EntityKind::Slingshot ||
        cb->GetEntityKind() == Character::EntityKind::Slingshot)
        return debugOut;

    auto apos = ca->GetPosition();
    auto bpos = cb->GetPosition();
    auto asz = ca->GetSize();
    auto bsz = cb->GetSize();
    auto avel = ca->GetVelocity();
    auto bvel = cb->GetVelocity();
    auto amass = ca->GetMass();
    auto bmass = cb->GetMass();
    auto arot = ca->m_Transform.rotation;
    auto brot = cb->m_Transform.rotation;

    const bool aStaticOnly = ca->IsStatic();
    const bool bStaticOnly = cb->IsStatic();

    // Wake threshold
    constexpr float kWakeVelThreshold = 8.0f;

    const glm::vec2 normal = contactNormal;
    const glm::vec2 comA = ca->GetCenterOfMassWorldPosition();
    const glm::vec2 comB = cb->GetCenterOfMassWorldPosition();
    const glm::vec2 rA = contactPoint - comA;
    const glm::vec2 rB = contactPoint - comB;

    const float invMassA = aStaticOnly ? 0.0f : 1.0f / std::max(0.0001f, amass);
    const float invMassB = bStaticOnly ? 0.0f : 1.0f / std::max(0.0001f, bmass);
    const float invMassSum = invMassA + invMassB;

    if (invMassSum > 0.0f)
    {
        const glm::vec2 relVel = bvel - avel;
        const float velAlongNormal = glm::dot(relVel, normal);

        if (velAlongNormal < 0.0f)
        {
            const float e = 0.5f * (CollisionUtils::GetRestitution(ca->GetMaterialType()) +
                                    CollisionUtils::GetRestitution(cb->GetMaterialType()));

            const float rectInertiaA = amass * (asz.x * asz.x + asz.y * asz.y) / 12.0f;
            const float rectInertiaB = bmass * (bsz.x * bsz.x + bsz.y * bsz.y) / 12.0f;
            constexpr float kInertiaFloorMultiplier = 4.0f;
            const float effectiveInertiaA = aStaticOnly ? FLT_MAX : std::max(ca->GetInertia(), rectInertiaA * kInertiaFloorMultiplier);
            const float effectiveInertiaB = bStaticOnly ? FLT_MAX : std::max(cb->GetInertia(), rectInertiaB * kInertiaFloorMultiplier);
            const float invInertiaA = aStaticOnly ? 0.0f : 1.0f / std::max(0.0001f, effectiveInertiaA);
            const float invInertiaB = bStaticOnly ? 0.0f : 1.0f / std::max(0.0001f, effectiveInertiaB);

            const float rnA = CollisionUtils::Cross(rA, normal);
            const float rnB = CollisionUtils::Cross(rB, normal);
            const float denom = invMassSum + rnA * rnA * invInertiaA + rnB * rnB * invInertiaB;
            const float j = (denom > 0.0f) ? (-(1.0f + e) * velAlongNormal / denom) : 0.0f;
            const glm::vec2 impulse = j * normal;

            // Friction
            glm::vec2 tangent = relVel - velAlongNormal * normal;
            float tangentLen = glm::length(tangent);
            glm::vec2 frictionImpulseVec{0.0f, 0.0f};
            if (tangentLen > 1e-6f)
            {
                tangent /= tangentLen;
                const float rtA = CollisionUtils::Cross(rA, tangent);
                const float rtB = CollisionUtils::Cross(rB, tangent);
                const float denomT = invMassSum + rtA * rtA * invInertiaA + rtB * rtB * invInertiaB;
                float jt = (denomT > 0.0f) ? (-glm::dot(relVel, tangent) / denomT) : 0.0f;

                const float mu = 0.5f * (CollisionUtils::GetFriction(ca->GetMaterialType()) + CollisionUtils::GetFriction(cb->GetMaterialType()));
                const float maxFriction = std::fabs(j) * mu;
                if (jt > maxFriction)
                    jt = maxFriction;
                else if (jt < -maxFriction)
                    jt = -maxFriction;

                frictionImpulseVec = jt * tangent;
            }

            if (ca->IsSleeping() && !aStaticOnly)
            {
                const glm::vec2 deltaV = -(impulse + frictionImpulseVec) * invMassA;
                if (glm::length(deltaV) > kWakeVelThreshold)
                {
                    ca->SetSleeping(false);
                }
            }
            if (cb->IsSleeping() && !bStaticOnly)
            {
                const glm::vec2 deltaV = (impulse + frictionImpulseVec) * invMassB;
                if (glm::length(deltaV) > kWakeVelThreshold)
                {
                    cb->SetSleeping(false);
                }
            }

            if (!aStaticOnly)
            {
                if (!ca->IsSleeping())
                {
                    ca->SetVelocity(avel - (impulse + frictionImpulseVec) * invMassA);
                    float torqueA = CollisionUtils::Cross(rA, -(impulse + frictionImpulseVec));
                    float deltaOmegaA = -torqueA * invInertiaA;
                    constexpr float kMaxDeltaOmega = 8.0f;
                    if (deltaOmegaA > kMaxDeltaOmega)
                        deltaOmegaA = kMaxDeltaOmega;
                    else if (deltaOmegaA < -kMaxDeltaOmega)
                        deltaOmegaA = -kMaxDeltaOmega;
                    ca->SetAngularVelocity(ca->GetAngularVelocity() + deltaOmegaA);
                }
            }
            if (!bStaticOnly)
            {
                if (!cb->IsSleeping())
                {
                    cb->SetVelocity(bvel + (impulse + frictionImpulseVec) * invMassB);
                    float torqueB = CollisionUtils::Cross(rB, (impulse + frictionImpulseVec));
                    float deltaOmegaB = torqueB * invInertiaB;
                    constexpr float kMaxDeltaOmega = 8.0f;
                    if (deltaOmegaB > kMaxDeltaOmega)
                        deltaOmegaB = kMaxDeltaOmega;
                    else if (deltaOmegaB < -kMaxDeltaOmega)
                        deltaOmegaB = -kMaxDeltaOmega;
                    cb->SetAngularVelocity(cb->GetAngularVelocity() + deltaOmegaB);
                }
            }
        }

        // Positional correction
        if (penetrationDepth > 0.0f && invMassSum > 0.0f)
        {
            const float kPercent = stabilizing ? 0.9f : 0.7f;
            const float kSlop = stabilizing ? 0.001f : 0.01f;
            const float correctionMag = std::max(penetrationDepth - kSlop, 0.0f) * kPercent / invMassSum;
            const glm::vec2 correction = correctionMag * normal;

            if (!aStaticOnly)
            {
                if (!ca->IsSleeping())
                    ca->SetPosition(ca->GetPosition() - correction * invMassA);
            }
            if (!bStaticOnly)
            {
                if (!cb->IsSleeping())
                    cb->SetPosition(cb->GetPosition() + correction * invMassB);
            }
        }
    }

    // Resting contact checks (kept here so ResolveCollision fully handles state)
    const float settleSpeedThreshold = 12.0f;
    const float settleAngularThreshold = 5.0f;
    const bool aIsIceLike = ca->GetMaterialType() == Character::MaterialType::Ice || ca->GetMaterialType() == Character::MaterialType::Glass;
    const bool bIsIceLike = cb->GetMaterialType() == Character::MaterialType::Ice || cb->GetMaterialType() == Character::MaterialType::Glass;
    const float iceSpeedThreshold = 8.0f;
    const float iceAngularThreshold = 3.0f;
    if (aStaticOnly && !bStaticOnly)
    {
        const float speedThreshold = bIsIceLike ? iceSpeedThreshold : settleSpeedThreshold;
        const float angularThreshold = bIsIceLike ? iceAngularThreshold : settleAngularThreshold;
        if (glm::length(cb->GetVelocity()) < speedThreshold && std::fabs(cb->GetAngularVelocity()) < angularThreshold)
        {
            DebugUtils::LogSleepDecision(cb->GetImagePath(), cb->GetPosition(), cb->GetVelocity(), cb->GetAngularVelocity(), "static_support_settled");
            cb->SetSleeping(true);
            cb->SetVelocity({0.0f, 0.0f});
            cb->SetAngularVelocity(0.0f);
        }
    }
    else if (bStaticOnly && !aStaticOnly)
    {
        const float speedThreshold = aIsIceLike ? iceSpeedThreshold : settleSpeedThreshold;
        const float angularThreshold = aIsIceLike ? iceAngularThreshold : settleAngularThreshold;
        if (glm::length(ca->GetVelocity()) < speedThreshold && std::fabs(ca->GetAngularVelocity()) < angularThreshold)
        {
            DebugUtils::LogSleepDecision(ca->GetImagePath(), ca->GetPosition(), ca->GetVelocity(), ca->GetAngularVelocity(), "static_support_settled");
            ca->SetSleeping(true);
            ca->SetVelocity({0.0f, 0.0f});
            ca->SetAngularVelocity(0.0f);
        }
    }

    // mutual rest
    const float kRestVelThreshold = 2.0f;
    const float kRestAngularThreshold = 1.0f;
    const float kSupportNormalY = 0.9f;
    if (!aStaticOnly && !bStaticOnly)
    {
        const float aRestVel = aIsIceLike ? 1.4f : kRestVelThreshold;
        const float bRestVel = bIsIceLike ? 1.4f : kRestVelThreshold;
        const float aRestAngular = aIsIceLike ? 0.8f : kRestAngularThreshold;
        const float bRestAngular = bIsIceLike ? 0.8f : kRestAngularThreshold;
        if (glm::length(ca->GetVelocity()) < aRestVel && glm::length(cb->GetVelocity()) < bRestVel &&
            std::fabs(ca->GetAngularVelocity()) < aRestAngular && std::fabs(cb->GetAngularVelocity()) < bRestAngular &&
            std::fabs(contactNormal.y) > kSupportNormalY && penetrationDepth > 0.0f)
        {
            DebugUtils::LogSleepDecision(ca->GetImagePath(), ca->GetPosition(), ca->GetVelocity(), ca->GetAngularVelocity(), "mutual_resting");
            DebugUtils::LogSleepDecision(cb->GetImagePath(), cb->GetPosition(), cb->GetVelocity(), cb->GetAngularVelocity(), "mutual_resting");
            ca->SetSleeping(true);
            cb->SetSleeping(true);
            ca->SetVelocity({0.0f, 0.0f});
            cb->SetVelocity({0.0f, 0.0f});
            ca->SetAngularVelocity(0.0f);
            cb->SetAngularVelocity(0.0f);
        }
    }

    // Prepare debug draw info for Scene to render
    DebugDrawInfo aInfo{apos, asz, arot, glm::vec4(1, 0, 0, 1), 0.2f};
    DebugDrawInfo bInfo{bpos, bsz, brot, glm::vec4(0, 0, 1, 1), 0.2f};
    debugOut.push_back(aInfo);
    debugOut.push_back(bInfo);

    return debugOut;
}
