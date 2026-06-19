#include "scenes/CollisionResponse.hpp"
#include "scenes/ContactManifold.hpp"
#include "scenes/CollisionUtils.hpp"
#include "entities/Character.hpp"
#include "scenes/DebugUtils.hpp"

#include <glm/glm.hpp>
#include <cfloat>
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────
// Internal helpers
// ─────────────────────────────────────────────
namespace
{
    // Relative velocity at the contact point, including angular contribution.
    // v_contact = v_center + ω × r   (in 2D: ω×r = vec2(-ω*r.y, ω*r.x))
    glm::vec2 VelAtContact(const Character *c, const glm::vec2 &r)
    {
        if (c->IsStatic() || c->IsSleeping())
            return {0.f, 0.f};
        float w = c->GetAngularVelocity();
        return c->GetVelocity() + glm::vec2(-w * r.y, w * r.x);
    }

    // Effective mass along direction d: 1/m + (r×d)²/I
    float EffMass(float invM, float invI, const glm::vec2 &r, const glm::vec2 &d)
    {
        float rCrossD = CollisionUtils::Cross(r, d);
        return invM + rCrossD * rCrossD * invI;
    }

    // Apply impulse P to character at lever arm r (P is applied in +P direction to body)
    void ApplyImpulse(Character *c, const glm::vec2 &r, const glm::vec2 &P,
                      float invM, float invI, float sign)
    {
        if (c->IsStatic() || c->IsSleeping())
            return;
        c->SetVelocity(c->GetVelocity() + sign * P * invM);
        float dw = sign * CollisionUtils::Cross(r, P) * invI;
        c->SetAngularVelocity(c->GetAngularVelocity() + dw);
    }

    // Per-body inertia using rect formula (no extra multiplier – pixels are large enough)
    float RectInertia(float mass, glm::vec2 size)
    {
        return mass * (size.x * size.x + size.y * size.y) / 12.f;
    }

    struct Bodies
    {
        Character *a;
        Character *b;
        bool aStatic;
        bool bStatic;
        float invMa;
        float invMb;
        float invIa;
        float invIb;
        glm::vec2 rA;
        glm::vec2 rB;
    };

    Bodies Setup(ContactManifold &cm)
    {
        Bodies o;
        o.a = cm.a;
        o.b = cm.b;
        o.aStatic = o.a->IsStatic();
        o.bStatic = o.b->IsStatic();

        float ma = o.a->GetMass(), mb = o.b->GetMass();
        float Ia = RectInertia(ma, o.a->GetSize());
        float Ib = RectInertia(mb, o.b->GetSize());
        Ia = std::max(o.a->GetInertia(), Ia);
        Ib = std::max(o.b->GetInertia(), Ib);

        o.invMa = o.aStatic ? 0.f : 1.f / std::max(0.0001f, ma);
        o.invMb = o.bStatic ? 0.f : 1.f / std::max(0.0001f, mb);
        o.invIa = o.aStatic ? 0.f : 1.f / std::max(0.0001f, Ia);
        o.invIb = o.bStatic ? 0.f : 1.f / std::max(0.0001f, Ib);

        glm::vec2 comA = o.a->GetCenterOfMassWorldPosition();
        glm::vec2 comB = o.b->GetCenterOfMassWorldPosition();
        o.rA = cm.contactPoint - comA;
        o.rB = cm.contactPoint - comB;
        return o;
    }

    // Wake an object if the velocity change is large enough
    void WakeCheck(Character *c, const glm::vec2 &dv, float invM)
    {
        // Threshold is 15.f (higher than gravity delta ~12.f)
        constexpr float kWakeThreshold = 15.f;
        if (!c->IsStatic() && c->IsSleeping())
        {
            if (glm::length(dv * invM) > kWakeThreshold)
                c->SetSleeping(false);
        }
    }

    // Sleep check: only allow sleeping on stable upward support from a static body
    void SleepCheck(Character *c, bool counterpartIsStatic, float supportNormalY, float penetration)
    {
        if (c->IsStatic() || c->IsSleeping())
            return;
        constexpr float kSleepV = 10.f;
        constexpr float kSleepW = 1.5f;
        constexpr float kSupportNormalY = 0.6f;
        constexpr float kMinSupportPenetration = 0.01f;

        if (!counterpartIsStatic)
            return;
        if (supportNormalY < kSupportNormalY)
            return;
        if (penetration < kMinSupportPenetration)
            return;

        // For elongated objects, don't allow sleep at unstable angles.
        // A rectangle is stable lying flat (0°) or upright (90°).
        // Sleeping at an intermediate angle causes the "stuck diagonal" artifact.
        {
            const glm::vec2 size = c->GetSize();
            const float longer  = std::max(size.x, size.y);
            const float shorter = std::min(size.x, size.y);
            const float aspect  = longer / std::max(0.01f, shorter);
            if (aspect > 1.4f)
            {
                constexpr float kHalfPi = 1.5707963f;
                constexpr float kPi     = 3.1415927f;
                float rot = std::fmod(c->m_Transform.rotation, kPi);
                if (rot >  kHalfPi) rot -= kPi;
                if (rot < -kHalfPi) rot += kPi;
                // Distance to nearest stable angle (0 or ±π/2)
                float nearest  = std::round(rot / kHalfPi) * kHalfPi;
                float angleOff = std::fabs(rot - nearest);
                // ~5° threshold — must be close to flat or upright to sleep
                if (angleOff > 0.09f)
                    return;
            }
        }

        if (glm::length(c->GetVelocity()) < kSleepV &&
            std::fabs(c->GetAngularVelocity()) < kSleepW)
        {
            DebugUtils::LogSleepDecision(c->GetImagePath(), c->GetPosition(),
                                         c->GetVelocity(), c->GetAngularVelocity(),
                                         "static_support_settled");
            c->SetSleeping(true);
            c->SetVelocity({0.f, 0.f});
            c->SetAngularVelocity(0.f);
        }
    }
} // anonymous namespace

// ─────────────────────────────────────────────
// WarmStart
// ─────────────────────────────────────────────
void CollisionResponse::WarmStart(ContactManifold &cm)
{
    if (cm.normalImpulse == 0.f && cm.tangentImpulse == 0.f)
        return;
    auto bd = Setup(cm);

    glm::vec2 P = cm.normalImpulse * cm.normal + cm.tangentImpulse * cm.tangent;

    // DO NOT WakeCheck here! Warm starting is just restoring the solver state
    // for resting contact. A resting contact has a continuous impulse to counteract
    // gravity (e.g., > 11.6 per frame). If we check it here, it will instantly wake
    // up sleeping objects every frame!

    ApplyImpulse(bd.a, bd.rA, P, bd.invMa, bd.invIa, -1.f);
    ApplyImpulse(bd.b, bd.rB, P, bd.invMb, bd.invIb, +1.f);
}

// ─────────────────────────────────────────────
// SolveVelocity
// ─────────────────────────────────────────────
void CollisionResponse::SolveVelocity(ContactManifold &cm, bool damageEnabled)
{
    auto bd = Setup(cm);
    float invMassSum = bd.invMa + bd.invMb;
    if (invMassSum == 0.f)
        return;

    // ── Normal impulse ────────────────────────────────────────────────────
    {
        glm::vec2 vA = VelAtContact(bd.a, bd.rA);
        glm::vec2 vB = VelAtContact(bd.b, bd.rB);
        glm::vec2 relVel = vB - vA;
        float velN = glm::dot(relVel, cm.normal);

        // Already separating – nothing to do
        if (velN >= 0.f)
            goto friction;

        float e = 0.5f * (CollisionUtils::GetRestitution(bd.a->GetMaterialType()) +
                          CollisionUtils::GetRestitution(bd.b->GetMaterialType()));
        // Zero out restitution for slow contacts to prevent micro-bounce.
        // Gravity applies ~11.6 px/s per frame, so threshold should be higher than that.
        if (std::fabs(velN) < 30.f)
            e = 0.f;

        float denom = EffMass(bd.invMa, bd.invIa, bd.rA, cm.normal) +
                      EffMass(bd.invMb, bd.invIb, bd.rB, cm.normal);
        if (denom <= 0.f)
            goto friction;

        float deltaJn = -(1.f + e) * velN / denom;

        // Accumulate and clamp (normal impulse >= 0)
        float jnOld = cm.normalImpulse;
        constexpr float kMaxNormalImpulse = 1200.0f;
        cm.normalImpulse = std::clamp(jnOld + deltaJn, 0.0f, kMaxNormalImpulse);
        float actualJn = cm.normalImpulse - jnOld;

        // Wake before applying
        glm::vec2 Pn = actualJn * cm.normal;
        WakeCheck(bd.a, -Pn, bd.invMa);
        WakeCheck(bd.b, Pn, bd.invMb);

        ApplyImpulse(bd.a, bd.rA, Pn, bd.invMa, bd.invIa, -1.f);
        ApplyImpulse(bd.b, bd.rB, Pn, bd.invMb, bd.invIb, +1.f);

        // Accumulate normal impulse magnitude for this contact over solver
        // iterations so damage can be applied once per physics step instead
        // of once per iteration (which would artificially multiply damage).
        if (damageEnabled)
        {
            cm.frameAccumulatedNormalImpulse += std::fabs(actualJn);
        }
    }

// ── Tangent (friction) impulse ────────────────────────────────────────
friction:
{
    glm::vec2 vA = VelAtContact(bd.a, bd.rA);
    glm::vec2 vB = VelAtContact(bd.b, bd.rB);
    glm::vec2 relVel = vB - vA;

    float velT = glm::dot(relVel, cm.tangent);

    float denom = EffMass(bd.invMa, bd.invIa, bd.rA, cm.tangent) +
                  EffMass(bd.invMb, bd.invIb, bd.rB, cm.tangent);
    if (denom <= 0.f)
        goto sleep_check;

    float deltaJt = -velT / denom;

    float mu = 0.5f * (CollisionUtils::GetFriction(bd.a->GetMaterialType()) +
                       CollisionUtils::GetFriction(bd.b->GetMaterialType()));
    float maxJt = mu * cm.normalImpulse;

    float jtOld = cm.tangentImpulse;
    cm.tangentImpulse = std::clamp(jtOld + deltaJt, -maxJt, maxJt);
    float actualJt = cm.tangentImpulse - jtOld;

    glm::vec2 Pt = actualJt * cm.tangent;
    ApplyImpulse(bd.a, bd.rA, Pt, bd.invMa, bd.invIa, -1.f);
    ApplyImpulse(bd.b, bd.rB, Pt, bd.invMb, bd.invIb, +1.f);
}

// ── Sleep check ───────────────────────────────────────────────────────
sleep_check:
    if (!bd.aStatic)
        SleepCheck(bd.a, bd.bStatic, -cm.normal.y, cm.penetration);
    if (!bd.bStatic)
        SleepCheck(bd.b, bd.aStatic, cm.normal.y, cm.penetration);
}

// ─────────────────────────────────────────────
// SolvePosition  (Baumgarte, velocity-independent)
// ─────────────────────────────────────────────
void CollisionResponse::SolvePosition(ContactManifold &cm)
{
    // Box2D default is 0.2. A lower value (0.15) makes objects push apart softer,
    // reducing the "bouncy" jitter feeling when heavy stacks push against each other.
    constexpr float kBeta = 0.08f;
    // Sleeping objects also get gentle position correction (much smaller beta).
    // This silently removes residual penetration left over from StabilizeEnvironment
    // so that when a sleeping body eventually wakes up there is no stored overlap
    // that would make it snap/explode outward.
    constexpr float kBetaSleep = 0.05f;
    constexpr float kSlop = 0.01f;

    if (cm.penetration <= kSlop)
        return;

    auto bd = Setup(cm);
    float invMassSum = bd.invMa + bd.invMb;
    if (invMassSum <= 0.f)
        return;

    // Choose beta per-body depending on sleep state
    bool aSleep = !bd.aStatic && bd.a->IsSleeping();
    bool bSleep = !bd.bStatic && bd.b->IsSleeping();

    // If BOTH sides are sleeping we still want to nudge them apart (very gently)
    float betaA = aSleep ? kBetaSleep : kBeta;
    float betaB = bSleep ? kBetaSleep : kBeta;

    float corrA = betaA * (cm.penetration - kSlop) * bd.invMa / invMassSum;
    float corrB = betaB * (cm.penetration - kSlop) * bd.invMb / invMassSum;

    if (!bd.aStatic)
        bd.a->SetPosition(bd.a->GetPosition() - corrA * cm.normal);
    if (!bd.bStatic)
        bd.b->SetPosition(bd.b->GetPosition() + corrB * cm.normal);
}

// Apply damage accumulated over the solver iterations for this contact once
// per physics step. This avoids multiplying damage by the number of iterations.
void CollisionResponse::ApplyAccumulatedDamage(ContactManifold &cm, float physicsScale)
{
    if (cm.frameAccumulatedNormalImpulse <= 0.f)
        return;

    // Un-scale the impulse to make damage resolution-independent
    float absJn = cm.frameAccumulatedNormalImpulse / (physicsScale > 0.f ? physicsScale : 1.0f);

    auto bd = Setup(cm);
    auto applyDmg = [&](Character *c, bool isStatic)
    {
        if (isStatic || c->GetEntityKind() == Character::EntityKind::Bird)
        {
            return;
        }

        float damageThreshold = 350.0f; // was 150.0f
        float damageFactor = 0.025f; // was 0.05f

        if (c->GetEntityKind() == Character::EntityKind::Pig || c->IsSpecialItem())
        {
            // Pigs and the level 3 smile target should reliably die from
            // meaningful impacts, even when the collision only topples them
            // instead of producing a huge structure-on-structure impulse.
            damageThreshold = 180.0f; // was 70.0f
            damageFactor = 0.05f; // was 0.11f
        }

        if (absJn <= damageThreshold)
        {
            return;
        }

        const float base = (absJn - damageThreshold) * damageFactor;
        float r = CollisionUtils::GetDamageResistance(c->GetMaterialType());
        c->ApplyDamage(base / r);
    };

    applyDmg(bd.a, bd.aStatic);
    applyDmg(bd.b, bd.bStatic);

    // Reset accumulator for next physics step
    cm.frameAccumulatedNormalImpulse = 0.f;
}
