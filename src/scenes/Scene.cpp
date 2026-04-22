#include "Scene.hpp"
#include "Character.hpp"
#include "scenes/CollisionUtils.hpp"
// Collision detection and input
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include "Util/DebugBox.hpp"
#include "Util/Time.hpp"

namespace
{
  constexpr bool kEnableCollisionLog = false;
  constexpr bool kEnableCollisionDebugBoxes = false;
}

void Scene::Init()
{
  if (m_BGM)
  {
    m_BGM->Play_BGM();
  }
  if (m_Background)
  {
    m_Background->Init();
  }
  for (auto &element : m_Elements)
  {
    element->Init();
  }

  // Run a short stabilization pass so environment pieces settle before gameplay
  StabilizeEnvironment(40);
}

void Scene::StabilizeEnvironment(int steps)
{
  if (steps <= 0)
    return;

  constexpr float localDt = 1.0f / 120.0f; // finer substeps for stabilization
  constexpr float gravity = 700.0f;
  const float settleSpeedThreshold = 6.0f;   // px/sec (stricter)
  const float settleAngularThreshold = 2.0f; // rad/sec
  // First, run a few positional-only relaxation passes with no gravity to remove
  // tiny initial overlaps caused by level placement rounding. This prevents gravity
  // from immediately driving pieces through small gaps and creating cascade collapse.
  constexpr int kInitialRelaxPasses = 12;
  RunCollisionDetection(8, true); // one initial pass to ensure basic corrections
  for (int r = 0; r < kInitialRelaxPasses; ++r)
  {
    RunCollisionDetection(8, true);
  }

  for (int s = 0; s < steps; ++s)
  {
    // Apply gravity and integrate only for non-static environment objects
    for (auto &element : m_Elements)
    {
      auto ch = std::dynamic_pointer_cast<Character>(element);
      if (!ch)
        continue;
      if (ch->GetEntityKind() != Character::EntityKind::Environment)
        continue;
      if (ch->IsStatic())
        continue;
      // apply gravity
      auto v = ch->GetVelocity();
      v.y -= gravity * localDt;
      ch->SetVelocity(v);
      ch->IntegratePhysics(localDt);
    }

    // Run multiple collision passes per substep to iteratively converge positional corrections.
    // Use a few more passes during stabilization to eliminate small gaps/penetrations.
    constexpr int kStabilizationPasses = 8;
    RunCollisionDetection(kStabilizationPasses, true);
  }

  // After stabilization steps, mark settled environment objects as sleeping
  for (auto &element : m_Elements)
  {
    auto ch = std::dynamic_pointer_cast<Character>(element);
    if (!ch)
      continue;
    if (ch->GetEntityKind() != Character::EntityKind::Environment)
      continue;
    if (ch->IsStatic())
      continue;
    if (glm::length(ch->GetVelocity()) < settleSpeedThreshold && std::fabs(ch->GetAngularVelocity()) < settleAngularThreshold)
    {
      ch->SetSleeping(true);
      ch->SetVelocity({0.0f, 0.0f});
      ch->SetAngularVelocity(0.0f);
    }
  }
}

void Scene::Update()
{
  // Get frame delta time from global Time (ms -> seconds)
  float deltaSec = std::max(0.0f, Util::Time::GetDeltaTimeMs() / 1000.0f);

  // Clamp to avoid spiral of death
  if (deltaSec > m_MaxFrameTime)
    deltaSec = m_MaxFrameTime;

  m_Accumulator += deltaSec;
  m_DebugDrawCooldown = std::max(0.0f, m_DebugDrawCooldown - deltaSec);

  int substeps = 0;
  while (m_Accumulator >= m_PhysicsStep && substeps < m_MaxSubSteps)
  {
    // Apply global gravity to all dynamic characters
    constexpr float kGlobalGravity = 700.0f;
    for (auto &element : m_Elements)
    {
      auto ch = std::dynamic_pointer_cast<Character>(element);
      if (!ch)
        continue;
      if (ch->IsStatic() || ch->IsSleeping())
        continue;
      auto v = ch->GetVelocity();
      v.y -= kGlobalGravity * m_PhysicsStep;
      ch->SetVelocity(v);
    }

    // Integrate physics for all characters
    for (auto &element : m_Elements)
    {
      auto ch = std::dynamic_pointer_cast<Character>(element);
      if (ch)
      {
        ch->IntegratePhysics(m_PhysicsStep);
      }
    }

    // Keep dynamic objects above the floor boundary and stop them there.
    for (auto &element : m_Elements)
    {
      auto ch = std::dynamic_pointer_cast<Character>(element);
      if (!ch || ch->IsStatic())
      {
        continue;
      }

      // Clamp using object's bottom edge (position.y - halfHeight)
      const float halfH = ch->GetSize().y * 0.5f;
      if (ch->GetPosition().y - halfH < this->m_WorldFloorY)
      {
        glm::vec2 pos = ch->GetPosition();
        pos.y = this->m_WorldFloorY + halfH;
        ch->SetPosition(pos);
        ch->SetVelocity({0.0f, 0.0f});
        ch->SetAngularVelocity(0.0f);
      }
    }

    // Run collision detection per physics step
    RunCollisionDetection();

    m_Accumulator -= m_PhysicsStep;
    ++substeps;
  }

  // Call per-frame update hooks and render/update children normally
  if (m_OnUpdate)
  {
    m_OnUpdate();
  }
  if (m_Background)
  {
    m_Background->Update();
  }
  for (auto &element : m_Elements)
  {
    element->Update();
  }
  // Update debug entities TTL and remove expired ones
  if (!m_DebugEntities.empty())
  {
    for (auto it = m_DebugEntities.begin(); it != m_DebugEntities.end();)
    {
      it->ttl -= deltaSec;
      if (it->ttl <= 0.0f)
      {
        // remove from m_Elements and from child list
        auto found = std::find(m_Elements.begin(), m_Elements.end(), it->obj);
        if (found != m_Elements.end())
        {
          m_Elements.erase(found);
        }
        RemoveChild(it->obj);
        it = m_DebugEntities.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }
  // Keyboard control for testing: move controlled character with arrow keys/WASD
  if (m_Controlled)
  {
    float speed = 5.0f; // units per update
    glm::vec2 pos = m_Controlled->GetPosition();
    if (Util::Input::IsKeyDown(Util::Keycode::LEFT) ||
        Util::Input::IsKeyDown(Util::Keycode::A))
    {
      pos.x -= speed;
    }
    if (Util::Input::IsKeyDown(Util::Keycode::RIGHT) ||
        Util::Input::IsKeyDown(Util::Keycode::D))
    {
      pos.x += speed;
    }
    if (Util::Input::IsKeyDown(Util::Keycode::UP) ||
        Util::Input::IsKeyDown(Util::Keycode::W))
    {
      pos.y -= speed;
    }
    if (Util::Input::IsKeyDown(Util::Keycode::DOWN) ||
        Util::Input::IsKeyDown(Util::Keycode::S))
    {
      pos.y += speed;
    }
    m_Controlled->SetPosition(pos);
  }
}

void Scene::RunCollisionDetection(int passes, bool stabilizing)
{
  if (passes <= 0)
    passes = 1;
  std::vector<std::shared_ptr<Character>> characters;
  characters.reserve(m_Elements.size());

  for (const auto &element : m_Elements)
  {
    auto ch = std::dynamic_pointer_cast<Character>(element);
    if (ch && ch->GetEntityKind() != Character::EntityKind::Slingshot)
    {
      characters.push_back(ch);
    }
  }

  for (int pass = 0; pass < passes; ++pass)
  {
    for (size_t i = 0; i < characters.size(); ++i)
    {
      for (size_t j = i + 1; j < characters.size(); ++j)
      {
        auto ca = characters[i];
        auto cb = characters[j];

        glm::vec2 contactNormal, contactPoint;
        float penetration = 0.0f;
        if (CollisionUtils::ComputeOBBMTV(*ca, *cb, contactNormal, penetration, contactPoint))
        {
          HandleCollision(ca, cb, contactNormal, penetration, contactPoint, stabilizing);
        }
      }
    }
  }
}

void Scene::HandleCollision(const std::shared_ptr<Util::GameObject> &a,
                            const std::shared_ptr<Util::GameObject> &b,
                            const glm::vec2 &contactNormal,
                            float penetrationDepth,
                            const glm::vec2 &contactPoint,
                            bool stabilizing)
{
  auto ca = std::dynamic_pointer_cast<Character>(a);
  auto cb = std::dynamic_pointer_cast<Character>(b);

  if (!(ca && cb))
    return;

  // Slingshot should be a pure visual/anchor object and must not block birds.
  if (ca->GetEntityKind() == Character::EntityKind::Slingshot ||
      cb->GetEntityKind() == Character::EntityKind::Slingshot)
  {
    return;
  }

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

  if (kEnableCollisionLog)
  {
    std::cout << "Collision detected:\n";
    std::cout << " A - pos(" << apos.x << "," << apos.y << ") size(" << asz.x << "," << asz.y << ") rot(" << arot << ") vel(" << avel.x << "," << avel.y << ") mass(" << amass << ")\n";
    std::cout << " B - pos(" << bpos.x << "," << bpos.y << ") size(" << bsz.x << "," << bsz.y << ") rot(" << brot << ") vel(" << bvel.x << "," << bvel.y << ") mass(" << bmass << ")\n";
  }

  // Linear impulse response (no angular impulse yet)
  const bool aStaticOnly = ca->IsStatic();
  const bool bStaticOnly = cb->IsStatic();

  // Wake threshold: if applied delta velocity from impulse exceeds this, wake a sleeping body
  constexpr float kWakeVelThreshold = 8.0f; // px/sec (raised to avoid waking on tiny impulses)

  // We will treat sleeping bodies as immobile for small impacts, but allow large impulses to wake them.
  {
    // Use the provided contact normal / penetration / contact point (MTV) computed by SAT.
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

        // Estimate a reasonable inertia lower bound from mass and size (rectangle about center)
        const float rectInertiaA = amass * (asz.x * asz.x + asz.y * asz.y) / 12.0f;
        const float rectInertiaB = bmass * (bsz.x * bsz.x + bsz.y * bsz.y) / 12.0f;
        constexpr float kInertiaFloorMultiplier = 2.0f;
        const float effectiveInertiaA = aStaticOnly ? FLT_MAX : std::max(ca->GetInertia(), rectInertiaA * kInertiaFloorMultiplier);
        const float effectiveInertiaB = bStaticOnly ? FLT_MAX : std::max(cb->GetInertia(), rectInertiaB * kInertiaFloorMultiplier);
        const float invInertiaA = aStaticOnly ? 0.0f : 1.0f / std::max(0.0001f, effectiveInertiaA);
        const float invInertiaB = bStaticOnly ? 0.0f : 1.0f / std::max(0.0001f, effectiveInertiaB);

        // denominator includes rotational effect
        const float rnA = CollisionUtils::Cross(rA, normal);
        const float rnB = CollisionUtils::Cross(rB, normal);
        const float denom = invMassSum + rnA * rnA * invInertiaA + rnB * rnB * invInertiaB;
        const float j = (denom > 0.0f) ? (-(1.0f + e) * velAlongNormal / denom) : 0.0f;
        const glm::vec2 impulse = j * normal;

        // Friction (Coulomb) along tangent
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

        // Wake sleeping bodies if delta v from total impulse is large
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

        // Apply linear impulse (normal + friction) and angular impulse (torque)
        if (!aStaticOnly)
        {
          if (!ca->IsSleeping())
          {
            ca->SetVelocity(avel - (impulse + frictionImpulseVec) * invMassA);
            float torqueA = CollisionUtils::Cross(rA, -(impulse + frictionImpulseVec));
            float deltaOmegaA = -torqueA * invInertiaA;
            constexpr float kMaxDeltaOmega = 8.0f; // rad/sec
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
            constexpr float kMaxDeltaOmega = 8.0f; // rad/sec
            if (deltaOmegaB > kMaxDeltaOmega)
              deltaOmegaB = kMaxDeltaOmega;
            else if (deltaOmegaB < -kMaxDeltaOmega)
              deltaOmegaB = -kMaxDeltaOmega;
            cb->SetAngularVelocity(cb->GetAngularVelocity() + deltaOmegaB);
          }
        }
      }

      // Positional correction along MTV to reduce interpenetration jitter.
      if (penetrationDepth > 0.0f && invMassSum > 0.0f)
      {
        // Use more aggressive correction during stabilization to close small gaps.
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
  }

  // If one body is static and the other is dynamic but relative speed is very small,
  // consider the dynamic body as settled and mark it static to avoid it sliding forever.
  const float settleSpeedThreshold = 12.0f;  // px/sec
  const float settleAngularThreshold = 5.0f; // rad/sec
  if (aStaticOnly && !bStaticOnly)
  {
    if (glm::length(cb->GetVelocity()) < settleSpeedThreshold && std::fabs(cb->GetAngularVelocity()) < settleAngularThreshold)
    {
      cb->SetSleeping(true);
      cb->SetVelocity({0.0f, 0.0f});
      cb->SetAngularVelocity(0.0f);
    }
  }
  else if (bStaticOnly && !aStaticOnly)
  {
    if (glm::length(ca->GetVelocity()) < settleSpeedThreshold && std::fabs(ca->GetAngularVelocity()) < settleAngularThreshold)
    {
      ca->SetSleeping(true);
      ca->SetVelocity({0.0f, 0.0f});
      ca->SetAngularVelocity(0.0f);
    }
  }

  // Additional resting contact detection for two dynamic bodies in near-vertical contact.
  // If both linear and angular velocities are tiny and the contact normal is nearly vertical,
  // treat them as resting and put them to sleep to emulate static friction/stability.
  const float kRestVelThreshold = 2.0f;     // px/sec
  const float kRestAngularThreshold = 1.0f; // rad/sec
  const float kSupportNormalY = 0.9f;       // normal must be mostly vertical
  if (!aStaticOnly && !bStaticOnly)
  {
    if (glm::length(ca->GetVelocity()) < kRestVelThreshold && glm::length(cb->GetVelocity()) < kRestVelThreshold &&
        std::fabs(ca->GetAngularVelocity()) < kRestAngularThreshold && std::fabs(cb->GetAngularVelocity()) < kRestAngularThreshold &&
        std::fabs(contactNormal.y) > kSupportNormalY && penetrationDepth > 0.0f)
    {
      ca->SetSleeping(true);
      cb->SetSleeping(true);
      ca->SetVelocity({0.0f, 0.0f});
      cb->SetVelocity({0.0f, 0.0f});
      ca->SetAngularVelocity(0.0f);
      cb->SetAngularVelocity(0.0f);
    }
  }

  // Render debug boxes for A and B by creating simple GameObjects with DebugBox drawables
  if (!kEnableCollisionDebugBoxes || m_DebugDrawCooldown > 0.0f)
  {
    return;
  }

  m_DebugDrawCooldown = m_DebugDrawInterval;

  try
  {
    auto debugA = std::make_shared<Util::DebugBox>(glm::vec4(1, 0, 0, 1), 0.02f);
    auto goA = std::make_shared<Util::GameObject>(debugA, 100.0f);
    goA->m_Transform.translation = apos;
    goA->m_Transform.rotation = arot;
    goA->m_Transform.scale = asz;
    AddElements(goA);
    m_DebugEntities.push_back({goA, 0.2f}); // short lifetime to reduce draw load

    auto debugB = std::make_shared<Util::DebugBox>(glm::vec4(0, 0, 1, 1), 0.02f);
    auto goB = std::make_shared<Util::GameObject>(debugB, 100.0f);
    goB->m_Transform.translation = bpos;
    goB->m_Transform.rotation = brot;
    goB->m_Transform.scale = bsz;
    AddElements(goB);
    m_DebugEntities.push_back({goB, 0.2f});
  }
  catch (...)
  {
    // If debug drawable isn't available for some reason, ignore.
  }
}