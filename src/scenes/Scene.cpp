#include "Scene.hpp"
#include "Character.hpp"
#include "scenes/CollisionUtils.hpp"
#include "scenes/CollisionResponse.hpp"
#include "scenes/DebugUtils.hpp"
#include "scenes/SleepSupport.hpp"
// Collision detection and input
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Resource.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include "Util/Time.hpp"

namespace
{
  constexpr bool kEnableCollisionDebugBoxes = false;

  // Build image ID with damage state suffix (_1, _2, _3, _4)
  // Undamaged state uses _1, subsequent damage states use _2, _3, _4, etc.
  std::string GetImageIdForDamageState(const std::string &baseImageId, Character::DamageState state)
  {
    // Extract base name without existing _1.._4 suffix
    std::string baseName = baseImageId;
    size_t lastUnderscore = baseName.rfind('_');
    if (lastUnderscore != std::string::npos)
    {
      const std::string suffix = baseName.substr(lastUnderscore + 1);
      // If last part is _1, _2, _3, or _4, remove it
      if (suffix.length() == 1 && suffix[0] >= '1' && suffix[0] <= '9')
      {
        baseName = baseName.substr(0, lastUnderscore);
      }
    }

    // Map damage state to image suffix
    // DamageState::Undamaged (0) -> _1 (first/undamaged variant)
    // DamageState::Light (1) -> _2, Moderate (2) -> _3, Heavy (3) -> _4, Critical (4) -> _5, etc.
    int imageSuffix = static_cast<int>(state) + 1;
    // If the generated id doesn't exist in resources, fallback to the highest
    // existing lower-numbered variant so we never return a non-existent id.
    std::string candidate = baseName + "_" + std::to_string(imageSuffix);
    while (imageSuffix > 1 && Resource::GetPath(candidate).empty())
    {
      imageSuffix -= 1;
      candidate = baseName + "_" + std::to_string(imageSuffix);
    }
    // If still not found, try the base id mapping (some resources map base id to _1)
    if (Resource::GetPath(candidate).empty())
    {
      if (!Resource::GetPath(baseName).empty())
        return baseName; // Resource maps baseName -> first variant
    }
    return candidate;
  }

  constexpr float settleSpeedThreshold = 6.0f;   // px/sec (stricter)
  constexpr float settleAngularThreshold = 2.0f; // rad/sec
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
  StabilizeEnvironment(120);
}

void Scene::StabilizeEnvironment(int steps)
{
  if (steps <= 0)
    return;

  constexpr float localDt = 1.0f / 120.0f; // finer substeps for stabilization
  constexpr float gravity = 700.0f;
  // First, run a few positional-only relaxation passes with no gravity to remove
  // tiny initial overlaps caused by level placement rounding. This prevents gravity
  // from immediately driving pieces through small gaps and creating cascade collapse.
  constexpr int kInitialRelaxPasses = 25; // more passes to clear initial overlaps first
  RunCollisionDetection(8, true);
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
      if (!ch->ParticipatesInPhysics())
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

  // Sleep and zero velocity only for environment objects that are below settle
  // thresholds after stabilization. Objects still above thresholds remain awake.
  // Impulse-based physics cannot perfectly converge to static equilibrium in a
  // finite number of steps, and the 2-second grace period
  // (m_DamageImmunityTimer) handles remaining micro-settling.
  for (auto &element : m_Elements)
  {
    auto ch = std::dynamic_pointer_cast<Character>(element);
    if (!ch)
      continue;
    if (!ch->ParticipatesInPhysics())
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

  m_Contacts.clear();
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
  // Count down the post-load grace period during which no damage is applied.
  if (m_DamageImmunityTimer > 0.0f)
    m_DamageImmunityTimer = std::max(0.0f, m_DamageImmunityTimer - deltaSec);

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
      if (!ch->ParticipatesInPhysics())
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
      if (ch && ch->ParticipatesInPhysics())
      {
        ch->IntegratePhysics(m_PhysicsStep);
      }
    }

    // Keep dynamic objects above the floor boundary and stop them there.
    for (auto &element : m_Elements)
    {
      auto ch = std::dynamic_pointer_cast<Character>(element);
      if (!ch || !ch->ParticipatesInPhysics() || ch->IsStatic())
      {
        continue;
      }

      // Clamp using the rotated OBB's lowest world-space point, not the unrotated center height.
      const glm::vec2 size = ch->GetSize();
      const float halfW = size.x * 0.5f;
      const float halfH = size.y * 0.5f;
      const float rot = ch->m_Transform.rotation;
      const float worldHalfY = std::fabs(std::sin(rot)) * halfW + std::fabs(std::cos(rot)) * halfH;

      if (ch->GetPosition().y - worldHalfY < this->m_WorldFloorY)
      {
        glm::vec2 pos = ch->GetPosition();
        pos.y = this->m_WorldFloorY + worldHalfY;
        ch->SetPosition(pos);

        glm::vec2 vel = ch->GetVelocity();
        if (vel.y < 0.0f)
          vel.y = 0.0f;

        // Apply ground friction when sliding on the global floor
        // (since it's a hardcoded boundary, not an OBB with friction)
        vel.x *= 0.95f;

        ch->SetVelocity(vel);

        // Add a slight angular damping when rolling on the global floor
        ch->SetAngularVelocity(ch->GetAngularVelocity() * 0.95f);
      }
    }

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

  // Update sprite images based on damage state
  for (auto &element : m_Elements)
  {
    auto character = std::dynamic_pointer_cast<Character>(element);
    if (!character || character->GetBaseImageId().empty())
      continue;

    const Character::DamageState currentState = character->GetDamageState();
    const Character::DamageState previousState = character->GetPreviousDamageState();

    // If damage state changed, update the image
    if (currentState != previousState)
    {
      character->SetPreviousDamageState(currentState);
      const std::string baseId = character->GetBaseImageId();
      const std::string newImageId = GetImageIdForDamageState(baseId, currentState);
      const std::string imagePath = Resource::GetPath(newImageId);

      if (!imagePath.empty())
      {
        character->SetImage(imagePath);
      }
    }
  }

  // Handle character deaths and award points
  for (auto it = m_Elements.begin(); it != m_Elements.end();)
  {
    auto character = std::dynamic_pointer_cast<Character>(*it);
    if (character && character->GetHealth() <= 0.0f && !character->IsDestroyed())
    {
      // Mark as destroyed once so we do not re-score every frame.
      character->SetDestroyed(true);

      // Award points based on character type
      const Character::EntityKind kind = character->GetEntityKind();
      if (kind == Character::EntityKind::Pig)
      {
        m_Score += 100; // Pig elimination bonus
        character->SetVisible(false);
        // TODO: Play pig death sound effect
      }
      else if (kind == Character::EntityKind::Environment)
      {
        m_Score += 10; // Structure destruction bonus
        // TODO: Play destruction sound effect
      }
      else
      {
        character->SetVisible(false);
      }

      // Notify subclasses about character death
      OnCharacterDeath(character);

      // Remove dead objects from the scene (Pigs and Environment blocks like wood/ice)
      // When their health reaches 0, they should shatter/disappear.
      RemoveChild(*it);
      it = m_Elements.erase(it);
    }
    else
    {
      ++it;
    }
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

  // ── Build character list ──────────────────────────────────────────────
  std::vector<std::shared_ptr<Character>> characters;
  characters.reserve(m_Elements.size());
  std::vector<std::shared_ptr<Character>> sleepingDynamics;
  std::vector<Character *> newlyActivated;
  auto supportConfirmed = SleepSupport::CreateSupportConfirmedSet();

  for (const auto &element : m_Elements)
  {
    auto ch = std::dynamic_pointer_cast<Character>(element);
    if (ch && ch->ParticipatesInPhysics() && ch->GetEntityKind() != Character::EntityKind::Slingshot)
    {
      characters.push_back(ch);
      if (ch->IsSleeping() && !ch->IsStatic())
        sleepingDynamics.push_back(ch);
    }
  }

  // ── BroadPhase + NarrowPhase: update ContactManifold list ─────────────
  // Mark all existing contacts as inactive; they'll be reactivated if still colliding.
  for (auto &cm : m_Contacts)
    cm.active = false;

  for (size_t i = 0; i < characters.size(); ++i)
  {
    for (size_t j = i + 1; j < characters.size(); ++j)
    {
      auto ca = characters[i];
      auto cb = characters[j];

      // Ignore collisions involving the Slingshot
      if (ca->GetEntityKind() == Character::EntityKind::Slingshot || cb->GetEntityKind() == Character::EntityKind::Slingshot)
        continue;

      // Ignore collisions between birds (e.g. active bird vs waiting birds)
      if (ca->GetEntityKind() == Character::EntityKind::Bird && cb->GetEntityKind() == Character::EntityKind::Bird)
        continue;

      glm::vec2 normal, contactPoint;
      float penetration = 0.f;
      if (!CollisionUtils::ComputeOBBMTV(*ca, *cb, normal, penetration, contactPoint))
        continue;

      // Impact activation check: wake environmental objects if hit by birds or already activated objects
      auto activateEnvironment = [&newlyActivated](const std::shared_ptr<Character> &target, const std::shared_ptr<Character> &other)
      {
        if (!target || target->GetEntityKind() != Character::EntityKind::Environment)
          return;
        if (target->GetMaterialType() == Character::MaterialType::Earth)
          return;
        if (target->IsImpactActivated())
          return;

        bool triggered = false;
        if (other->GetEntityKind() == Character::EntityKind::Bird)
        {
          triggered = true;
        }
        else if (other->GetEntityKind() == Character::EntityKind::Environment)
        {
          // Can transfer impact if other is already activated and not sleeping
          triggered = other->IsImpactActivated() && !other->IsSleeping();
        }

        if (triggered)
        {
          target->SetImpactActivated(true);
          target->SetStatic(false);
          target->SetSleeping(false);
          target->SetVelocity({0.0f, 0.0f});
          target->SetAngularVelocity(0.0f);
          newlyActivated.push_back(target.get());
        }
      };
      activateEnvironment(ca, cb);
      activateEnvironment(cb, ca);

      // Sleep support tracking
      if (ca->IsSleeping() && !ca->IsStatic() &&
          SleepSupport::IsBottomSupportContact(ca, cb, contactPoint))
        supportConfirmed.insert(ca.get());
      if (cb->IsSleeping() && !cb->IsStatic() &&
          SleepSupport::IsBottomSupportContact(cb, ca, contactPoint))
        supportConfirmed.insert(cb.get());

      // Find or create manifold for this pair
      ContactManifold *found = nullptr;
      for (auto &cm : m_Contacts)
      {
        if (cm.Matches(ca.get(), cb.get()))
        {
          found = &cm;
          break;
        }
      }

      if (!found)
      {
        // New contact – create fresh manifold (no warm-start yet)
        ContactManifold cm;
        cm.a = ca.get();
        cm.b = cb.get();
        m_Contacts.push_back(cm);
        found = &m_Contacts.back();
      }

      // If contact was stored in reversed order, flip accumulated impulses
      bool flip = !found->SameOrder(ca.get(), cb.get());

      found->a = ca.get();
      found->b = cb.get();
      found->normal = normal; // always a→b
      found->tangent = glm::vec2(-normal.y, normal.x);
      found->contactPoint = contactPoint;
      found->penetration = penetration;
      found->active = true;
      if (flip)
      {
        // Normal flipped – negate accumulated tangent (friction reversed too)
        found->normalImpulse = found->normalImpulse; // stays positive
        found->tangentImpulse = -found->tangentImpulse;
      }
    }
  }

  // Remove stale (no-longer-colliding) contacts and zero their impulses
  m_Contacts.erase(
      std::remove_if(m_Contacts.begin(), m_Contacts.end(),
                     [](const ContactManifold &cm)
                     { return !cm.active; }),
      m_Contacts.end());

  if (!newlyActivated.empty())
  {
    for (auto &cm : m_Contacts)
    {
      if (std::find(newlyActivated.begin(), newlyActivated.end(), cm.a) != newlyActivated.end() ||
          std::find(newlyActivated.begin(), newlyActivated.end(), cm.b) != newlyActivated.end())
      {
        cm.normalImpulse = 0.0f;
        cm.tangentImpulse = 0.0f;
        cm.frameAccumulatedNormalImpulse = 0.0f;
      }
    }
  }

  // ── Solve ──────────────────────────────────────────────────────────────
  // We ALWAYS need to run velocity solver, even during stabilizing,
  // otherwise gravity keeps accelerating objects and they explode.

  for (auto &cm : m_Contacts)
    CollisionResponse::WarmStart(cm);

  const bool damageEnabled = !stabilizing && !IsDamageImmune();
  constexpr int kVelIterations = 10;
  for (int iter = 0; iter < kVelIterations; ++iter)
    for (auto &cm : m_Contacts)
      CollisionResponse::SolveVelocity(cm, damageEnabled);

  // Apply accumulated damage once per contact for this physics step.
  if (damageEnabled)
  {
    for (auto &cm : m_Contacts)
    {
      CollisionResponse::ApplyAccumulatedDamage(cm);
    }
  }

  constexpr int kPosIterations = 3;
  for (int iter = 0; iter < kPosIterations; ++iter)
    for (auto &cm : m_Contacts)
      CollisionResponse::SolvePosition(cm);

  if (stabilizing)
  {
    // During stabilizing, we just want to settle, no need to wake things up yet
  }
  else
  {
    // Wake sleeping objects that have lost geometric support
    for (const auto &ch : sleepingDynamics)
    {
      if (ch->GetEntityKind() == Character::EntityKind::Environment && !ch->IsImpactActivated())
        continue;
      if (supportConfirmed.find(ch.get()) != supportConfirmed.end())
        continue;
      if (SleepSupport::IsGeometricallySupported(ch, m_Elements, m_WorldFloorY))
        continue;

      DebugUtils::LogSleepDecision(ch->GetImagePath(), ch->GetPosition(),
                                   ch->GetVelocity(), ch->GetAngularVelocity(),
                                   "lost_support_wake");
      ch->SetSleeping(false);
    }
  }
}
