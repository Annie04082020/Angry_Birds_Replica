#include "Scene.hpp"
#include "Character.hpp"
#include "scenes/CollisionUtils.hpp"
#include "scenes/CollisionResponse.hpp"
#include "scenes/DebugUtils.hpp"
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
  constexpr bool kEnableCollisionDebugBoxes = false;
}

void Scene::AddDebugEntity(const std::shared_ptr<Util::GameObject> &obj, float ttl)
{
  if (!obj)
    return;
  AddElements(obj);
  m_DebugEntities.push_back({obj, ttl});
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
      DebugUtils::LogSleepDecision(ch->GetImagePath(), ch->GetPosition(), ch->GetVelocity(), ch->GetAngularVelocity(), "stabilize_settled");
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
  // Delegate physics resolution to the CollisionResponse helper which returns debug draw entries.
  auto ca = std::dynamic_pointer_cast<Character>(a);
  auto cb = std::dynamic_pointer_cast<Character>(b);
  if (!(ca && cb))
    return;

  if (ca->GetEntityKind() == Character::EntityKind::Slingshot || cb->GetEntityKind() == Character::EntityKind::Slingshot)
    return;

  auto debugInfos = CollisionResponse::ResolveCollision(ca, cb, contactNormal, penetrationDepth, contactPoint, stabilizing);

  if (!debugInfos.empty() && kEnableCollisionDebugBoxes && m_DebugDrawCooldown <= 0.0f)
  {
    m_DebugDrawCooldown = m_DebugDrawInterval;
    try
    {
      for (const auto &info : debugInfos)
      {
        auto debugDraw = std::make_shared<Util::DebugBox>(info.color, 0.02f);
        auto go = std::make_shared<Util::GameObject>(debugDraw, 100.0f);
        go->m_Transform.translation = info.pos;
        go->m_Transform.rotation = info.rotation;
        go->m_Transform.scale = info.scale;
        AddElements(go);
        m_DebugEntities.push_back({go, info.ttl});
      }
    }
    catch (...)
    {
    }
  }
}