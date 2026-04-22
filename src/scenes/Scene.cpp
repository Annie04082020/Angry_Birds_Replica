#include "Scene.hpp"
// Collision detection and input
#include "Character.hpp"
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
  constexpr float kWorldFloorY = -320.0f;

  float GetRestitution(Character::MaterialType mat)
  {
    switch (mat)
    {
    case Character::MaterialType::Flesh:
      return 0.2f;
    case Character::MaterialType::Wood:
      return 0.12f;
    case Character::MaterialType::Stone:
      return 0.06f;
    case Character::MaterialType::Glass:
    case Character::MaterialType::Ice:
      return 0.18f;
    default:
      return 0.1f;
    }
  }
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

      if (ch->GetPosition().y < kWorldFloorY)
      {
        glm::vec2 pos = ch->GetPosition();
        pos.y = kWorldFloorY;
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

void Scene::RunCollisionDetection()
{
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

  for (size_t i = 0; i < characters.size(); ++i)
  {
    for (size_t j = i + 1; j < characters.size(); ++j)
    {
      auto ca = characters[i];
      auto cb = characters[j];

      if (ca->IfCollides(cb))
      {
        HandleCollision(ca, cb);
      }
    }
  }
}

void Scene::HandleCollision(const std::shared_ptr<Util::GameObject> &a,
                            const std::shared_ptr<Util::GameObject> &b)
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
  constexpr float kWakeVelThreshold = 5.0f; // px/sec

  // We will treat sleeping bodies as immobile for small impacts, but allow large impulses to wake them.
  {
    glm::vec2 normal = bpos - apos;
    const float lenSq = glm::dot(normal, normal);
    if (lenSq > 1e-6f)
    {
      normal /= std::sqrt(lenSq);
    }
    else
    {
      normal = glm::vec2(1.0f, 0.0f);
    }

    const float invMassA = aStaticOnly ? 0.0f : 1.0f / std::max(0.0001f, amass);
    const float invMassB = bStaticOnly ? 0.0f : 1.0f / std::max(0.0001f, bmass);
    const float invMassSum = invMassA + invMassB;

    if (invMassSum > 0.0f)
    {
      const glm::vec2 relVel = bvel - avel;
      const float velAlongNormal = glm::dot(relVel, normal);

      if (velAlongNormal < 0.0f)
      {
        const float e = 0.5f * (GetRestitution(ca->GetMaterialType()) +
                                GetRestitution(cb->GetMaterialType()));
        const float j = -(1.0f + e) * velAlongNormal / invMassSum;
        const glm::vec2 impulse = j * normal;

        // If either body is sleeping, check whether this impulse would wake it (delta-v > threshold).
        if (ca->IsSleeping() && !aStaticOnly)
        {
          const glm::vec2 deltaV = -impulse * invMassA;
          if (glm::length(deltaV) > kWakeVelThreshold)
          {
            ca->SetSleeping(false);
          }
        }
        if (cb->IsSleeping() && !bStaticOnly)
        {
          const glm::vec2 deltaV = impulse * invMassB;
          if (glm::length(deltaV) > kWakeVelThreshold)
          {
            cb->SetSleeping(false);
          }
        }

        // Apply impulse to non-static, non-sleeping bodies. Sleeping bodies with small delta remain unaffected.
        if (!aStaticOnly)
        {
          if (!ca->IsSleeping())
          {
            ca->SetVelocity(avel - impulse * invMassA);
          }
        }
        if (!bStaticOnly)
        {
          if (!cb->IsSleeping())
          {
            cb->SetVelocity(bvel + impulse * invMassB);
          }
        }
      }

      // Positional correction to reduce interpenetration jitter.
      const float dx = bpos.x - apos.x;
      const float dy = bpos.y - apos.y;
      const float overlapX = (asz.x * 0.5f + bsz.x * 0.5f) - std::fabs(dx);
      const float overlapY = (asz.y * 0.5f + bsz.y * 0.5f) - std::fabs(dy);

      if (overlapX > 0.0f && overlapY > 0.0f)
      {
        float penetration = overlapX;
        glm::vec2 correctionNormal = glm::vec2((dx >= 0.0f) ? 1.0f : -1.0f, 0.0f);
        if (overlapY < overlapX)
        {
          penetration = overlapY;
          correctionNormal = glm::vec2(0.0f, (dy >= 0.0f) ? 1.0f : -1.0f);
        }

        constexpr float kPercent = 0.7f;
        constexpr float kSlop = 0.01f;
        const float correctionMag = std::max(penetration - kSlop, 0.0f) *
                                    kPercent / invMassSum;
        const glm::vec2 correction = correctionMag * correctionNormal;

        if (!aStaticOnly)
        {
          // If the body is sleeping we avoid nudging it; otherwise apply correction.
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
  const float settleSpeedThreshold = 12.0f; // px/sec
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