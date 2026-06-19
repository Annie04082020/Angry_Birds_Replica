#include "BirdLaunchController.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <glm/glm.hpp>
#include <random>

#include "SoundEffect.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Time.hpp"
#include "Util/TransformUtils.hpp"
#include "Util/ViewportUtils.hpp"

namespace {
constexpr float kBirdReadyDistanceThreshold = 6.0f;
constexpr float kIdleHopHeight = 32.0f;
constexpr float kIdleHopDuration = 0.42f;
constexpr float kIdleFlipDuration = 0.58f;
constexpr float kFullRotation = 6.2831853f;
constexpr float kSettledBirdRemovalDelay = 2.0f;

std::mt19937 &GetIdleAnimationRng() {
  static std::mt19937 rng{std::random_device{}()};
  return rng;
}

float NextIdleActionCooldown() {
  static std::uniform_real_distribution<float> cooldownDist(0.85f, 1.9f);
  return cooldownDist(GetIdleAnimationRng());
}

float NextBirdVocalCooldown() {
  static std::uniform_real_distribution<float> cooldownDist(1.0f, 2.4f);
  return cooldownDist(GetIdleAnimationRng());
}

float NextPigVocalCooldown() {
  static std::uniform_real_distribution<float> cooldownDist(1.6f, 2.4f);
  return cooldownDist(GetIdleAnimationRng());
}

bool ShouldPlayBirdVocal() {
  static std::bernoulli_distribution playDist(0.75);
  return playDist(GetIdleAnimationRng());
}

bool ShouldPlayPigVocal() {
  static std::bernoulli_distribution playDist(0.65);
  return playDist(GetIdleAnimationRng());
}

size_t NextBirdVocalIndex(const size_t vocalCount) {
  if (vocalCount <= 1) {
    return 0;
  }

  std::uniform_int_distribution<size_t> indexDist(0, vocalCount - 1);
  return indexDist(GetIdleAnimationRng());
}

BirdLaunchController::IdleActionType NextIdleActionType() {
  static std::uniform_int_distribution<int> actionDist(0, 2);
  switch (actionDist(GetIdleAnimationRng())) {
  case 0:
    return BirdLaunchController::IdleActionType::Hop;
  case 1:
    return BirdLaunchController::IdleActionType::ForwardFlip;
  default:
    return BirdLaunchController::IdleActionType::BackwardFlip;
  }
}

float EvaluateHopYOffset(const float normalizedTime) {
  const float clamped = std::clamp(normalizedTime, 0.0f, 1.0f);
  return std::sin(clamped * 3.1415926f) * kIdleHopHeight;
}

std::string ToLowerCopy(std::string value) {
  std::transform(
      value.begin(), value.end(), value.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}
} // namespace

bool BirdLaunchController::LoadLevelObjects(
    const std::vector<std::shared_ptr<Character>> &objects) {
  m_BirdQueue.clear();
  m_IdlePigs.clear();
  m_ActiveBird = nullptr;
  m_CurrentBirdIndex = 0;
  m_IsHoldingBird = false;
  m_HasLaunchedBird = false;
  m_HasAnyBirdBeenLaunched = false;
  m_BirdVelocity = {0.0f, 0.0f};
  m_ActiveBirdsInFlight.clear();
  m_HasSplit = false;
  m_LaunchSequence = 0;
  m_ActiveBirdBaseRotation = 0.0f;
  m_QueuedBirdIdleStates.clear();
  m_SettledBirdRemovalTimers.clear();
  m_SettledBirdsPendingRemoval.clear();
  m_PigVocalCooldowns.clear();
  m_BirdIdleVocalSfx.clear();
  m_PigIdleVocalSfx.clear();
  for (int i = 1; i <= 12; ++i) {
    m_BirdIdleVocalSfx.push_back(std::make_shared<SoundEffect>(
        std::string(RESOURCE_DIR) + "/Audio/SFX/bird misc a" +
        std::to_string(i) + ".wav"));
    m_PigIdleVocalSfx.push_back(std::make_shared<SoundEffect>(
        std::string(RESOURCE_DIR) + "/Audio/SFX/piglette oink a" +
        std::to_string(i) + ".wav"));
  }
  m_ActiveBirdVocalCooldown = NextBirdVocalCooldown();

  m_Slingshots.clear();

  for (const auto &obj : objects) {
    if (!obj) {
      continue;
    }

    const std::string pathLower = ToLowerCopy(obj->GetImagePath());
    if (pathLower.find("/image/birds/") != std::string::npos) {
      m_BirdQueue.push_back(obj);
      continue;
    }

    if (pathLower.find("/image/pigs/") != std::string::npos) {
      m_IdlePigs.push_back(obj);
      m_PigVocalCooldowns[obj.get()] = NextPigVocalCooldown();
      continue;
    }

    if (pathLower.find("slingshot") != std::string::npos ||
        pathLower.find("sprite_147") != std::string::npos ||
        pathLower.find("sprite_154") != std::string::npos) {
      m_Slingshots.push_back(obj);
    }
  }

  if (!m_Slingshots.empty()) {
    glm::vec2 center{0.0f, 0.0f};
    for (const auto &sling : m_Slingshots) {
      center += sling->GetPosition();
    }
    center /= static_cast<float>(m_Slingshots.size());
    m_BirdAnchorPosition = center + glm::vec2(0.0f, 90.0f * m_PhysicsScale);
  }

  if (!m_BirdQueue.empty()) {
    for (const auto &bird : m_BirdQueue) {
      if (!bird) {
        continue;
      }
      bird->SetVelocity({0.0f, 0.0f});
      bird->SetAngularVelocity(0.0f);
      bird->SetStatic(true);
      bird->SetParticipatesInPhysics(false);
    }
    ActivateBirdByIndex(0);
  }

  return true;
}

bool BirdLaunchController::Update() {
  const float dt = std::max(0.0f, Util::Time::GetDeltaTimeMs() / 1000.0f);
  m_ActiveBirdVocalCooldown = std::max(0.0f, m_ActiveBirdVocalCooldown - dt);
  UpdateSettledBirdRemoval(dt);
  UpdateQueuedBirdIdleAnimations(dt);
  UpdatePigIdleVocals(dt);
  return HandleBirdLaunchPhysics();
}

int BirdLaunchController::GetRemainingBirdCountForBonus() const {
  if (m_BirdQueue.empty()) {
    return 0;
  }

  int remaining = 0;
  const bool activeBirdReadyToLaunch =
      m_ActiveBird && !m_HasLaunchedBird &&
      m_CurrentBirdIndex < m_BirdQueue.size() &&
      glm::distance(m_ActiveBird->GetPosition(), m_BirdAnchorPosition) <=
          kBirdReadyDistanceThreshold;

  if (activeBirdReadyToLaunch) {
    ++remaining;
  }

  if (m_CurrentBirdIndex + 1 < m_BirdQueue.size()) {
    remaining +=
        static_cast<int>(m_BirdQueue.size() - (m_CurrentBirdIndex + 1));
  }

  return remaining;
}

std::vector<glm::vec2>
BirdLaunchController::GetRemainingBirdPositionsForBonus() const {
  std::vector<glm::vec2> positions;
  if (m_BirdQueue.empty()) {
    return positions;
  }

  const bool activeBirdReadyToLaunch =
      m_ActiveBird && !m_HasLaunchedBird &&
      m_CurrentBirdIndex < m_BirdQueue.size() &&
      glm::distance(m_ActiveBird->GetPosition(), m_BirdAnchorPosition) <=
          kBirdReadyDistanceThreshold;

  if (activeBirdReadyToLaunch) {
    positions.push_back(m_ActiveBird->GetPosition());
  }

  for (size_t i = m_CurrentBirdIndex + 1; i < m_BirdQueue.size(); ++i) {
    if (m_BirdQueue[i]) {
      positions.push_back(m_BirdQueue[i]->GetPosition());
    }
  }

  return positions;
}

bool BirdLaunchController::IsOutOfBirds() const {
  return m_HasAnyBirdBeenLaunched && !m_HasLaunchedBird && !m_IsHoldingBird &&
         GetRemainingBirdCountForBonus() == 0;
}

glm::vec2 BirdLaunchController::GetMouseWorldPosition() const {
  const glm::vec2 mousePos = Util::Input::GetCursorPosition();
  const float zoom = Util::GetCameraZoom();
  const glm::vec2 cameraPos = Util::GetCameraPosition();
  return mousePos / zoom + cameraPos;
}

bool BirdLaunchController::HandleBirdLaunchPhysics() {
  if (!m_ActiveBird) {
    return false;
  }

  const float dt = std::max(0.0f, Util::Time::GetDeltaTimeMs() / 1000.0f);
  const bool mousePressed = Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB);
  const glm::vec2 mouseWorldPos = GetMouseWorldPosition();

  const float maxPullDistance = 140.0f * m_PhysicsScale;
  const float launchPower =
      10.5f; // Do not scale launchPower, as pull distance is already scaled!
  if (!m_HasLaunchedBird) {
    if (!m_IsHoldingBird && m_ActiveBird && m_ActiveBirdVocalCooldown <= 0.0f &&
        ShouldPlayBirdVocal()) {
      PlayRandomBirdVocal();
      m_ActiveBirdVocalCooldown = NextBirdVocalCooldown();
    }

    if (mousePressed) {
      if (!m_IsHoldingBird && m_ActiveBird->IsHovering(mouseWorldPos)) {
        m_IsHoldingBird = true;
      }

      if (m_IsHoldingBird) {
        glm::vec2 pull = mouseWorldPos - m_BirdAnchorPosition;
        const float pullLength = glm::length(pull);
        if (pullLength > maxPullDistance && pullLength > 0.0f) {
          pull = pull / pullLength * maxPullDistance;
        }

        m_ActiveBird->SetPosition(m_BirdAnchorPosition + pull);
        m_ActiveBird->SetVelocity({0.0f, 0.0f});
        m_BirdVelocity = {0.0f, 0.0f};
        return true;
      }
    } else if (m_IsHoldingBird) {
      const glm::vec2 pullVector =
          m_ActiveBird->GetPosition() - m_BirdAnchorPosition;
      // Scale launch velocity by mass so heavier/lighter birds behave
      // consistently.
      const float birdMass = std::max(0.0001f, m_ActiveBird->GetMass());
      const float massScale =
          std::sqrt(1.0f / birdMass); // keep kinetic energy roughly similar
      m_BirdVelocity = -pullVector * launchPower * massScale;
      m_ActiveBird->SetRotation(m_ActiveBirdBaseRotation);
      m_ActiveBird->SetStatic(false);
      m_ActiveBird->SetParticipatesInPhysics(true);
      m_ActiveBird->SetVelocity(m_BirdVelocity);
      m_IsHoldingBird = false;
      m_HasLaunchedBird = true;
      m_HasAnyBirdBeenLaunched = true;
      ++m_LaunchSequence;
      m_ActiveBirdsInFlight.clear();
      m_ActiveBirdsInFlight.push_back(m_ActiveBird);
      m_HasSplit = false;
      return true;
    }

    return m_IsHoldingBird;
  }

  // Check for split trigger if it is a blue bird and has not split yet
  const std::string imgPath = ToLowerCopy(m_ActiveBird->GetImagePath());
  const bool isBlueBird = (m_ActiveBird->GetBaseImageId() == "BIRD_BLUE" ||
                           imgPath.find("blue_birds") != std::string::npos);
  const bool triggerSplit =
      isBlueBird && !m_HasSplit &&
      (Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB) ||
       Util::Input::IsKeyPressed(Util::Keycode::SPACE));
  if (triggerSplit) {
    m_HasSplit = true;

    auto cloneBird = [](const std::shared_ptr<Character> &original,
                        float angleOffsetDeg) {
      auto clone = std::make_shared<Character>(original->GetImagePath());
      clone->SetBaseImageId(original->GetBaseImageId());
      clone->SetScale(original->GetTransform().scale);
      clone->SetRotation(original->GetTransform().rotation);
      clone->SetZIndex(original->GetZIndex());
      clone->SetEntityKind(original->GetEntityKind());
      clone->SetMaterialType(original->GetMaterialType());
      clone->SetColliderShape(original->GetColliderShape());
      clone->SetMass(original->GetMass());
      clone->SetInertia(original->GetInertia());
      clone->SetHealth(original->GetHealth());
      clone->SetMaxHealth(original->GetMaxHealth());
      clone->SetNumDamageStates(original->GetNumDamageStates());
      clone->SetStatic(false);
      clone->SetParticipatesInPhysics(true);
      clone->SetImpactActivated(true);

      glm::vec2 originalPos = original->GetPosition();
      glm::vec2 originalVel = original->GetVelocity();

      float angleRad = glm::radians(angleOffsetDeg);
      float cosA = std::cos(angleRad);
      float sinA = std::sin(angleRad);
      glm::vec2 rotatedVel = {originalVel.x * cosA - originalVel.y * sinA,
                              originalVel.x * sinA + originalVel.y * cosA};

      glm::vec2 perpDir = {0.0f, 1.0f};
      if (glm::length(originalVel) > 0.001f) {
        glm::vec2 normalizedVel = glm::normalize(originalVel);
        perpDir = {-normalizedVel.y, normalizedVel.x};
      }
      clone->SetPosition(originalPos +
                         perpDir * (angleOffsetDeg > 0.0f ? 30.0f : -30.0f));
      clone->SetVelocity(rotatedVel);
      return clone;
    };

    auto upperBird = cloneBird(m_ActiveBird, 15.0f);
    auto lowerBird = cloneBird(m_ActiveBird, -15.0f);

    if (m_OnSpawnCharacter) {
      m_OnSpawnCharacter(upperBird);
      m_OnSpawnCharacter(lowerBird);
    }

    m_ActiveBirdsInFlight.push_back(upperBird);
    m_ActiveBirdsInFlight.push_back(lowerBird);
  }

  bool allBirdsStopped = true;
  const float stopSpeedThreshold = 10.0f;  // px/sec
  const float stopAngularThreshold = 5.0f; // rad/sec

  for (auto &bird : m_ActiveBirdsInFlight) {
    if (!bird)
      continue;

    if (bird->IsStatic() || bird->IsSleeping()) {
      ScheduleSettledBirdRemoval(bird);
      continue;
    }

    glm::vec2 vel = bird->GetVelocity();
    glm::vec2 nextPos = bird->GetPosition() + vel * dt;
    const float halfH = bird->GetSize().y * 0.5f;

    // Check bottom edge contact (center.y - halfHeight)
    if (nextPos.y - halfH < m_WorldFloorY) {
      nextPos.y = m_WorldFloorY + halfH;
      bird->SetPosition(nextPos);
      bird->SetVelocity({0.0f, 0.0f});
      bird->SetAngularVelocity(0.0f);
      bird->SetSleeping(true);
      bird->SetParticipatesInPhysics(false);
      ScheduleSettledBirdRemoval(bird);
      continue;
    }

    // Check if this bird has come to rest
    if (glm::length(vel) < stopSpeedThreshold &&
        std::fabs(bird->GetAngularVelocity()) < stopAngularThreshold) {
      bird->SetSleeping(true);
      bird->SetVelocity({0.0f, 0.0f});
      bird->SetAngularVelocity(0.0f);
      bird->SetParticipatesInPhysics(false);
      ScheduleSettledBirdRemoval(bird);
      continue;
    }

    // If we reach here, at least one bird is still flying
    allBirdsStopped = false;
  }

  if (allBirdsStopped) {
    m_HasLaunchedBird = false;
    m_ActiveBirdsInFlight.clear();
    m_BirdVelocity = {0.0f, 0.0f};
    if (m_CurrentBirdIndex + 1 < m_BirdQueue.size()) {
      ActivateBirdByIndex(m_CurrentBirdIndex + 1);
    } else {
      m_ActiveBird = nullptr;
    }
  }

  return true;
}

void BirdLaunchController::ActivateBirdByIndex(size_t index) {
  if (index >= m_BirdQueue.size()) {
    return;
  }

  m_CurrentBirdIndex = index;
  m_ActiveBird = m_BirdQueue[index];
  if (!m_ActiveBird) {
    return;
  }

  m_ActiveBird->SetPosition(m_BirdAnchorPosition);
  m_ActiveBird->SetVelocity({0.0f, 0.0f});
  m_ActiveBird->SetAngularVelocity(0.0f);
  m_ActiveBird->SetStatic(true);
  m_ActiveBird->SetParticipatesInPhysics(false);
  m_BirdVelocity = {0.0f, 0.0f};
  m_IsHoldingBird = false;
  m_HasLaunchedBird = false;
  m_ActiveBird->SetZIndex(0.0f);
  m_ActiveBirdsInFlight.clear();
  m_HasSplit = false;
  m_ActiveBirdBaseRotation = m_ActiveBird->GetTransform().rotation;
  m_ActiveBird->SetRotation(m_ActiveBirdBaseRotation);
  m_ActiveBirdVocalCooldown = NextBirdVocalCooldown();
}

void BirdLaunchController::UpdateSettledBirdRemoval(float deltaTimeSeconds) {
  if (m_SettledBirdsPendingRemoval.empty()) {
    return;
  }

  const float dt = std::max(0.0f, deltaTimeSeconds);
  for (const auto &bird : m_SettledBirdsPendingRemoval) {
    if (!bird) {
      continue;
    }

    float &timer = m_SettledBirdRemovalTimers[bird.get()];
    timer += dt;
  }

  m_SettledBirdsPendingRemoval.erase(
      std::remove_if(m_SettledBirdsPendingRemoval.begin(),
                     m_SettledBirdsPendingRemoval.end(),
                     [this](const std::shared_ptr<Character> &bird) {
                       if (!bird) {
                         return true;
                       }

                       const auto timerIt =
                           m_SettledBirdRemovalTimers.find(bird.get());
                       if (timerIt == m_SettledBirdRemovalTimers.end() ||
                           timerIt->second < kSettledBirdRemovalDelay) {
                         return false;
                       }

                       if (m_OnRemoveCharacter) {
                         m_OnRemoveCharacter(bird);
                       }
                       m_SettledBirdRemovalTimers.erase(timerIt);
                       return true;
                     }),
      m_SettledBirdsPendingRemoval.end());
}

void BirdLaunchController::ScheduleSettledBirdRemoval(
    const std::shared_ptr<Character> &bird) {
  if (!bird) {
    return;
  }

  if (m_SettledBirdRemovalTimers.find(bird.get()) !=
      m_SettledBirdRemovalTimers.end()) {
    return;
  }

  bird->SetVelocity({0.0f, 0.0f});
  bird->SetAngularVelocity(0.0f);
  bird->SetSleeping(true);
  bird->SetParticipatesInPhysics(false);
  m_SettledBirdRemovalTimers[bird.get()] = 0.0f;
  m_SettledBirdsPendingRemoval.push_back(bird);
}

void BirdLaunchController::UpdateQueuedBirdIdleAnimations(
    float deltaTimeSeconds) {
  if (m_BirdQueue.empty()) {
    return;
  }

  const float dt = std::max(0.0f, deltaTimeSeconds);

  for (size_t i = m_CurrentBirdIndex + 1; i < m_BirdQueue.size(); ++i) {
    const auto &bird = m_BirdQueue[i];
    if (!bird) {
      continue;
    }

    IdleAnimationState &state = m_QueuedBirdIdleStates[bird.get()];
    if (state.actionCooldown <= 0.0f && state.actionDuration <= 0.0f) {
      state.basePosition = bird->GetPosition();
      state.baseRotation = bird->GetTransform().rotation;
      state.actionCooldown = NextIdleActionCooldown();
      state.vocalCooldown = NextBirdVocalCooldown();
    }

    state.actionTimer += dt;
    state.vocalCooldown = std::max(0.0f, state.vocalCooldown - dt);

    if (state.actionType == IdleActionType::None) {
      if (state.actionTimer >= state.actionCooldown) {
        state.actionType = NextIdleActionType();
        state.actionTimer = 0.0f;
        state.actionDuration = (state.actionType == IdleActionType::Hop)
                                   ? kIdleHopDuration
                                   : kIdleFlipDuration;
        if (state.vocalCooldown <= 0.0f && ShouldPlayBirdVocal()) {
          PlayRandomBirdVocal();
          state.vocalCooldown = NextBirdVocalCooldown();
        }
      }

      bird->SetPosition(state.basePosition);
      bird->SetRotation(state.baseRotation);
      bird->SetVelocity({0.0f, 0.0f});
      bird->SetAngularVelocity(0.0f);
      continue;
    }

    const float actionDuration = std::max(0.0001f, state.actionDuration);
    const float progress =
        std::clamp(state.actionTimer / actionDuration, 0.0f, 1.0f);
    const float hopYOffset = EvaluateHopYOffset(progress) * m_PhysicsScale;

    float rotationOffset = 0.0f;
    if (state.actionType == IdleActionType::ForwardFlip) {
      rotationOffset = progress * kFullRotation;
    } else if (state.actionType == IdleActionType::BackwardFlip) {
      rotationOffset = -progress * kFullRotation;
    }

    bird->SetPosition(state.basePosition + glm::vec2{0.0f, hopYOffset});
    bird->SetRotation(state.baseRotation + rotationOffset);
    bird->SetVelocity({0.0f, 0.0f});
    bird->SetAngularVelocity(0.0f);

    if (progress >= 1.0f) {
      state.actionType = IdleActionType::None;
      state.actionTimer = 0.0f;
      state.actionDuration = 0.0f;
      state.actionCooldown = NextIdleActionCooldown();
      bird->SetPosition(state.basePosition);
      bird->SetRotation(state.baseRotation);
    }
  }
}

void BirdLaunchController::ResetQueuedBirdIdleAnimation(
    const std::shared_ptr<Character> &bird, bool snapToBasePosition) {
  if (!bird) {
    return;
  }

  auto it = m_QueuedBirdIdleStates.find(bird.get());
  if (it == m_QueuedBirdIdleStates.end()) {
    return;
  }

  it->second.actionTimer = 0.0f;
  it->second.actionCooldown = NextIdleActionCooldown();
  it->second.actionDuration = 0.0f;
  it->second.actionType = IdleActionType::None;
  it->second.vocalCooldown = NextBirdVocalCooldown();

  if (snapToBasePosition) {
    bird->SetPosition(it->second.basePosition);
  }
  bird->SetRotation(it->second.baseRotation);
}

void BirdLaunchController::PlayRandomBirdVocal() {
  if (m_BirdIdleVocalSfx.empty()) {
    return;
  }

  const size_t index = NextBirdVocalIndex(m_BirdIdleVocalSfx.size());
  const auto &sfx = m_BirdIdleVocalSfx[index];
  if (sfx) {
    sfx->Play_SFX();
  }
}

void BirdLaunchController::UpdatePigIdleVocals(float deltaTimeSeconds) {
  if (m_IdlePigs.empty()) {
    return;
  }

  m_IdlePigs.erase(std::remove_if(m_IdlePigs.begin(), m_IdlePigs.end(),
                                  [](const std::shared_ptr<Character> &pig) {
                                    return !pig || pig->IsDestroyed() ||
                                           pig->GetHealth() <= 0.0f;
                                  }),
                   m_IdlePigs.end());

  if (m_IdlePigs.empty()) {
    return;
  }

  const float dt = std::max(0.0f, deltaTimeSeconds);
  for (const auto &pig : m_IdlePigs) {

    float &cooldown = m_PigVocalCooldowns[pig.get()];
    if (cooldown <= 0.0f) {
      cooldown = NextPigVocalCooldown();
    }

    cooldown = std::max(0.0f, cooldown - dt);
    if (cooldown <= 0.0f && ShouldPlayPigVocal()) {
      PlayRandomPigVocal();
      cooldown = NextPigVocalCooldown();
    }
  }
}

void BirdLaunchController::PlayRandomPigVocal() {
  if (m_PigIdleVocalSfx.empty()) {
    return;
  }

  const size_t index = NextBirdVocalIndex(m_PigIdleVocalSfx.size());
  const auto &sfx = m_PigIdleVocalSfx[index];
  if (sfx) {
    sfx->Play_SFX();
  }
}
