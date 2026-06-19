#include "GameScene.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>
#include <unordered_map>

#include "config.hpp"

#include "Resource.hpp"
#include "Util/Color.hpp"
#include "Util/DebugBox.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Text.hpp"
#include "Util/Time.hpp"
#include "Util/TransformUtils.hpp"

namespace {
constexpr const char *kUIFont = RESOURCE_DIR "/font/angrybirds-regular.ttf";
constexpr const char *kHighScoreFilePath = RESOURCE_DIR "/high_scores.json";
constexpr const char *kBirdTrailDotImage =
    RESOURCE_DIR "/Image/assets/sprite_022.png";
constexpr const char *kBirdTrailHeadImage =
    RESOURCE_DIR "/Image/assets/sprite_032.png";
constexpr float kGrassTopRatio = 404.0f / 563.0f;
constexpr int kBirdTrailDotPoolSize = 140;
constexpr float kBirdTrailDotScale = 0.82f;
constexpr float kBirdTrailDotLifetime = 0.85f;
constexpr float kBirdTrailEmitDistance = 18.0f;
constexpr float kBirdTrailMinSpeed = 120.0f;
constexpr float kLeftoverBirdAwardInterval = 0.62f;
std::string GetHudLabel(const std::shared_ptr<Character> &object) {
  if (!object) {
    return "OBJECT";
  }

  std::string label = object->GetBaseImageId().empty()
                          ? object->GetImagePath()
                          : object->GetBaseImageId();
  const std::size_t slashPos = label.find_last_of("/\\");
  if (slashPos != std::string::npos) {
    label = label.substr(slashPos + 1);
  }

  const std::size_t dotPos = label.find_last_of('.');
  if (dotPos != std::string::npos) {
    label = label.substr(0, dotPos);
  }

  return label.empty() ? "OBJECT" : label;
}

std::string
BuildDamageHudText(const std::vector<std::shared_ptr<Character>> &objects) {
  std::ostringstream out;
  out << "Damage test\n";

  std::unordered_map<std::string, int> occurrenceCount;
  for (const auto &object : objects) {
    if (!object) {
      continue;
    }

    const std::string baseId = GetHudLabel(object);
    const int currentIndex = ++occurrenceCount[baseId];
    const float maxHealth = object->GetMaxHealth();
    const float health = object->GetHealth();
    const float damagePercent =
        maxHealth > 0.0f
            ? std::max(0.0f, (maxHealth - health) / maxHealth * 100.0f)
            : 0.0f;

    out << currentIndex << ". " << baseId
        << "  dmg: " << static_cast<int>(std::round(damagePercent)) << "%\n";
  }

  return out.str();
}

// Design resolution: all HUD constants below are authored for this resolution.
constexpr float kDesignWidth = 2400.0f;
constexpr float kDesignHeight = 1350.0f;

// Returns the scale factor to adapt design-resolution HUD values to the actual
// viewport.
float GetHudScale() {
  const glm::vec2 viewportSize = Util::GetViewportSize();
  if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f)
    return 1.0f;
  return std::min(viewportSize.x / kDesignWidth,
                  viewportSize.y / kDesignHeight);
}

std::string FormatScore(const int score) {
  std::ostringstream stream;
  stream << score;
  return stream.str();
}

bool IsLevelThreeSmileTarget(const LevelManager *levelManager,
                             const std::shared_ptr<Character> &character) {
  return levelManager && levelManager->GetLevel() == 3 && character &&
         character->IsSpecialItem() && character->GetBaseImageId() == "SMILE";
}
} // namespace

bool GameScene::LoadLevel(const std::string &levelPath) {
  m_ZoomScrollAccumulator = 0.0f;
  m_DamageOutputTimer = 0.0f;
  m_ShowDamageHud = false;
  m_DamageImmunityTimer = 2.0f;

  m_IsIntroAnimating = true;
  m_IntroTimer = 0.0f;
  m_IntroCameraTargetX = 0.0f;
  m_IntroWaitDuration = 1.0f;
  m_IntroDuration = 2.0f;

  Util::SetCameraZoom(1.0f);
  Util::SetCameraPosition({0.0f, 0.0f});

  const float physicsScale = GetHudScale();
  SetPhysicsScale(physicsScale);
  if (m_BirdLaunchController) {
    m_BirdLaunchController->SetPhysicsScale(physicsScale);
  }

  if (!m_LevelManager || !m_LevelManager->LoadLevel(levelPath)) {
    return false;
  }

  const auto &objects = m_LevelManager->GetGameObjects();

  m_IntroCameraTargetX = 0.0f;
  for (const auto &obj : objects) {
    if (obj && (obj->GetEntityKind() == Character::EntityKind::Pig ||
                obj->GetEntityKind() == Character::EntityKind::Environment)) {
      float x = obj->GetTransform().translation.x;
      if (x > m_IntroCameraTargetX) {
        m_IntroCameraTargetX = x;
      }
    }
    AddElements(obj);
  }

  // Set initial camera to look at the pigs on the right
  float initialCameraX = std::max(0.0f, m_IntroCameraTargetX - 600.0f);
  Util::SetCameraPosition({initialCameraX, 0.0f});

  const glm::vec2 viewportSize = Util::GetViewportSize();
  const float floorCandidate = (0.5f - kGrassTopRatio) * viewportSize.y;
  this->SetWorldFloorY(floorCandidate);

  if (m_BirdLaunchController) {
    m_BirdLaunchController->SetWorldFloorY(floorCandidate);
    m_BirdLaunchController->SetOnSpawnCharacter(
        [this](const std::shared_ptr<Character> &charPtr) {
          this->AddElements(charPtr);
        });
    m_BirdLaunchController->LoadLevelObjects(objects);
  }

  const bool isDamageTestLevel =
      levelPath.find("_test") != std::string::npos ||
      m_LevelManager->GetLevelName().find("Test") != std::string::npos;
  if (isDamageTestLevel) {
    m_ShowDamageHud = true;
    LOG_DEBUG("Test Level Loaded: {}\n{}", m_LevelManager->GetLevelName(),
              BuildDamageHudText(objects));
  }

  m_ScoringSystem.LoadConfig(RESOURCE_DIR "/scoring_config.json");
  LoadLevelHighScore();

  ResetScoreState();
  BuildLevelHud();
  m_BirdTrail.Build([this](const std::shared_ptr<Util::GameObject> &element) {
    AddElements(element);
  });
  m_BirdTrail.Reset();
  UpdateHudPositions();
  UpdateScoreHud();

  m_SceneInputController = std::make_shared<SceneInputController>(
      m_DynamicBackground, m_LevelManager);

  return true;
}

void GameScene::LoadLevelHighScore() {
  if (!m_LevelManager) {
    m_ScoringSystem.SetHighScore(0);
    m_HudHighScore = 0;
    return;
  }

  m_ScoringSystem.LoadHighScoreFromFile(kHighScoreFilePath,
                                        m_LevelManager->GetLevel());
  m_HudHighScore = m_ScoringSystem.GetHighScore();
}

void GameScene::PersistLevelHighScore() const {
  if (!m_LevelManager) {
    return;
  }

  m_ScoringSystem.SaveHighScoreToFile(kHighScoreFilePath,
                                      m_LevelManager->GetLevel());
}

void GameScene::Update() {
  if (Util::Input::IsKeyUp(Util::Keycode::C)) {
    m_LevelCleared = true;
    m_LevelFailed = false;
  }

  const bool isBirdHolding =
      m_BirdLaunchController && m_BirdLaunchController->IsHoldingBird();

  if (Util::Input::IsKeyPressed(Util::Keycode::F1)) {
    SetDebugRenderEnabled(!IsDebugRenderEnabled());
    LOG_DEBUG("Physics render {}",
              IsDebugRenderEnabled() ? "enabled" : "disabled");
  }

  if (Util::Input::IsKeyPressed(Util::Keycode::P)) {
    SetPhysicsPaused(!IsPhysicsPaused());
    LOG_DEBUG("Physics {}", IsPhysicsPaused() ? "paused" : "resumed");
  }

  if (IsPhysicsPaused() && Util::Input::IsKeyPressed(Util::Keycode::L)) {
    RequestPhysicsSingleStep();
    LOG_DEBUG("Physics single step dt=0.016");
  }

  // Output damage stats to console periodically during test level
  if (m_ShowDamageHud && m_LevelManager) {
    m_DamageOutputTimer += Util::Time::GetDeltaTimeMs() / 1000.0f;
    if (m_DamageOutputTimer >= 2.0f) // Output every 2 seconds
    {
      m_DamageOutputTimer = 0.0f;
      LOG_DEBUG("Damage Status:\n{}",
                BuildDamageHudText(m_LevelManager->GetGameObjects()));
    }
  }

  if (m_IsIntroAnimating) {
    float deltaTime = Util::Time::GetDeltaTimeMs() / 1000.0f;
    m_IntroTimer += deltaTime;

    if (m_IntroTimer > m_IntroWaitDuration) {
      float progress = (m_IntroTimer - m_IntroWaitDuration) / m_IntroDuration;
      if (progress >= 1.0f) {
        progress = 1.0f;
        m_IsIntroAnimating = false;
      }

      // Smoothstep easing
      float t = progress;
      float smoothProgress = t * t * (3.0f - 2.0f * t);

      float startX = std::max(0.0f, m_IntroCameraTargetX - 600.0f);
      float currentX = startX * (1.0f - smoothProgress);
      Util::SetCameraPosition({currentX, 0.0f});
    }
  }

  if (m_BirdLaunchController && !IsPhysicsPaused() && !m_IsIntroAnimating) {
    m_BirdLaunchController->Update();

    // Keep structures immune to self-inflicted collision damage until the
    // player actually fires the first bird. This prevents stacked levels
    // from breaking during initial settling.
    if (!m_BirdLaunchController->HasAnyBirdBeenLaunched()) {
      m_DamageImmunityTimer = std::max(m_DamageImmunityTimer, 0.1f);
    } else {
      m_DamageImmunityTimer = 0.0f;
    }
  }
  m_BirdTrail.Update(m_BirdLaunchController);

  if (m_SceneInputController && !m_IsIntroAnimating) {
    m_SceneInputController->Update(isBirdHolding);
  }

  if (m_PauseMenuInputBlockedUntilRelease &&
      !Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
    m_PauseMenuInputBlockedUntilRelease = false;
    if (m_PauseMenu) {
      m_PauseMenu->SetButtonsInputEnabled(true);
    }
  }

  // Handle mouse wheel zoom with mouse position as pivot
  if (Util::Input::IfScroll() && !m_IsIntroAnimating) {
    const glm::vec2 mousePos = Util::Input::GetCursorPosition();
    const float oldZoom = Util::GetCameraZoom();
    const glm::vec2 oldCameraPos = Util::GetCameraPosition();
    const glm::vec2 scrollDist = Util::Input::GetScrollDistance();
    const float normalizedScroll = std::clamp(scrollDist.y, -1.0f, 1.0f);
    m_ZoomScrollAccumulator += normalizedScroll;

    const int zoomSteps = static_cast<int>(m_ZoomScrollAccumulator);
    if (zoomSteps != 0) {
      m_ZoomScrollAccumulator -= static_cast<float>(zoomSteps);

      // Calculate world position of mouse before zoom
      const glm::vec2 worldMousePos = mousePos / oldZoom + oldCameraPos;

      constexpr float zoomStep = 0.01f;
      const float stepScale =
          zoomSteps > 0 ? (1.0f + zoomStep) : (1.0f - zoomStep);
      const float newZoom = oldZoom * std::pow(stepScale, std::abs(zoomSteps));
      Util::SetCameraZoom(newZoom);

      // Get actual zoom after clamp
      const float actualZoom = Util::GetCameraZoom();

      // Only adjust camera position if zoom actually changed (not clamped)
      if (actualZoom != oldZoom) {
        const glm::vec2 newCameraPos = worldMousePos - mousePos / actualZoom;
        glm::vec2 clampedCameraPos = newCameraPos;
        // Keep background bottom aligned with viewport bottom.
        clampedCameraPos.y = 0.0f;
        Util::SetCameraPosition(clampedCameraPos);
      }
    }
  }

  UpdateHudPositions();
  Scene::Update();
  RefreshRemainingPigCount();
  UpdateWinState();
  UpdateFailState();
}

void GameScene::BuildLevelHud() {
  m_GameHud.SetOnTogglePause([this]() { TogglePauseMenu(); });
  m_GameHud.SetOnRestart([this]() {
    SetPauseMenuVisible(false);
    if (m_OnRestartLevel) {
      m_OnRestartLevel();
    }
  });
  m_GameHud.Build(
      [this](const std::shared_ptr<Util::GameObject> &element) {
        AddElements(element);
      },
      GetHudScale());

  if (!m_PauseMenu) {
    m_PauseMenu = std::make_shared<PauseMenu>();
    m_PauseMenu->SetOnClose([this]() { SetPauseMenuVisible(false); });
    m_PauseMenu->SetOnRestart([this]() {
      SetPauseMenuVisible(false);
      if (m_OnRestartLevel) {
        m_OnRestartLevel();
      }
    });
    m_PauseMenu->SetOnOpenLevelSelect([this]() {
      SetPauseMenuVisible(false);
      if (m_OnOpenLevelSelect) {
        m_OnOpenLevelSelect();
      }
    });
    m_PauseMenu->SetOnToggleMute([this]() { ToggleMusicMute(); });
    m_PauseMenu->Build(
        [this](const std::shared_ptr<Util::GameObject> &element) {
          AddElements(element);
        },
        GetHudScale());
  }

  m_LevelResultPanel.SetOnOpenLevelSelect([this]() {
    if (m_OnOpenLevelSelect) {
      m_OnOpenLevelSelect();
    }
  });
  m_LevelResultPanel.SetOnRestart([this]() {
    if (m_OnRestartLevel) {
      m_OnRestartLevel();
    }
  });
  m_LevelResultPanel.SetOnNextLevel([this]() {
    if (m_OnNextLevel) {
      m_OnNextLevel();
    }
  });
  m_LevelResultPanel.Build(
      [this](const std::shared_ptr<Util::GameObject> &element) {
        AddElements(element);
      },
      GetHudScale());

  SetPauseMenuVisible(false);
}

void GameScene::UpdateHudPositions() {
  const glm::vec2 cameraPos = Util::GetCameraPosition();
  const glm::vec2 viewportSize = Util::GetViewportSize();
  const float zoom = Util::GetCameraZoom();
  const float hudScale = GetHudScale();
  m_GameHud.UpdateLayout(cameraPos, viewportSize, hudScale, zoom);

  if (m_PauseMenu) {
    m_PauseMenu->UpdateLayout(cameraPos, viewportSize, hudScale, zoom);
  }
}

void GameScene::SetPauseMenuVisible(const bool visible) {
  m_IsPauseMenuVisible = visible;
  m_GameHud.SetControlsVisible(!visible && !m_IsLevelClearScreenVisible &&
                               !m_IsLevelFailedScreenVisible);

  if (m_PauseMenu) {
    const int levelNumber = m_LevelManager ? m_LevelManager->GetLevel() : 0;
    m_PauseMenu->SetVisible(visible, m_IsMusicMuted, levelNumber);
  }

  if (visible) {
    m_PauseMenuInputBlockedUntilRelease = true;
    if (m_PauseMenu) {
      m_PauseMenu->SetButtonsInputEnabled(false);
    }
  } else {
    m_PauseMenuInputBlockedUntilRelease = false;
    if (m_PauseMenu) {
      m_PauseMenu->SetButtonsInputEnabled(true);
    }
  }
}

void GameScene::UpdateScoreHud() {
  m_GameHud.SetScore(m_ScoringSystem.GetScore());
  m_GameHud.SetHighScore(m_HudHighScore);
}

void GameScene::ResetScoreState() {
  m_ScoringSystem.Reset();
  m_RemainingPigCount = 0;
  m_LevelCleared = false;
  m_LevelFailed = false;
  m_LeftoverBirdsAwarded = false;
  m_PendingLeftoverBirdAwards = 0;
  m_PendingLeftoverBirdAwardPositions.clear();
  m_LeftoverBirdAwardTimer = 0.0f;
  m_LevelClearAnimationTime = 0.0f;
  m_IsLevelClearScreenVisible = false;
  m_IsLevelFailedScreenVisible = false;
  m_LevelResultPanel.Reset();

  for (const auto &object : m_LevelManager->GetGameObjects()) {
    if (!object) {
      continue;
    }

    object->SetDestroyed(false);
    object->ResetHealth();
    object->SetVisible(true);
    // Only set physics participation for objects that should participate
    // DECOR (Unknown kind) and Slingshot should not participate in physics
    if (object->GetEntityKind() == Character::EntityKind::Environment ||
        object->GetEntityKind() == Character::EntityKind::Pig) {
      object->SetParticipatesInPhysics(true);
    }

    if (object->GetEntityKind() == Character::EntityKind::Pig ||
        IsLevelThreeSmileTarget(m_LevelManager.get(), object)) {
      ++m_RemainingPigCount;
    }

    if (object->GetEntityKind() == Character::EntityKind::Environment &&
        !object->IsSpecialItem()) {
      object->SetScoreBudgetRemaining(
          m_ScoringSystem.GetDamageBudget(object->GetMaterialType()));
      object->SetOnDamageCallback([this](Character *c, float appliedDamage) {
        const float damageRatio =
            appliedDamage / std::max(0.0001f, c->GetMaxHealth());
        const int estimated =
            std::max(10, static_cast<int>(std::lround(
                             static_cast<float>(m_ScoringSystem.GetDamageBudget(
                                 c->GetMaterialType())) *
                             std::clamp(damageRatio, 0.0f, 1.0f))));
        const int awarded = c->DrainScoreBudget(estimated);
        if (awarded > 0) {
          const int actualScore = m_ScoringSystem.AwardBlockDamage(
              c->GetMaterialType(), damageRatio, awarded);
          SpawnFloatingScore(c->GetPosition(), actualScore,
                             Util::Color::FromRGB(255, 255, 255));
          UpdateScoreHud();
        }
      });
    } else {
      object->SetScoreBudgetRemaining(0);
      object->SetOnDamageCallback(nullptr);
    }
  }
}

void GameScene::RefreshRemainingPigCount() {
  if (!m_LevelManager) {
    m_RemainingPigCount = 0;
    return;
  }

  int alivePigCount = 0;
  for (const auto &object : m_LevelManager->GetGameObjects()) {
    if (!object) {
      continue;
    }

    if (object->GetEntityKind() != Character::EntityKind::Pig &&
        !IsLevelThreeSmileTarget(m_LevelManager.get(), object)) {
      continue;
    }

    if (object->GetHealth() > 0.0f && !object->IsDestroyed()) {
      ++alivePigCount;
    }
  }

  m_RemainingPigCount = alivePigCount;
  if (m_RemainingPigCount == 0) {
    m_LevelCleared = true;
    m_LevelFailed = false;
  }
}

void GameScene::UpdateWinState()
{
    if (!m_LevelCleared || m_LevelFailed)
    {
        return;
    }

    if (!m_IsLevelClearScreenVisible)
    {
        if (!m_LeftoverBirdsAwarded && m_BirdLaunchController)
        {
            const auto remainingBirdPositions = m_BirdLaunchController->GetRemainingBirdPositionsForBonus();
            m_PendingLeftoverBirdAwards = static_cast<int>(remainingBirdPositions.size());
            m_PendingLeftoverBirdAwardPositions.clear();
            for (const auto &position : remainingBirdPositions)
            {
                m_PendingLeftoverBirdAwardPositions.push_back(position);
            }
            m_LeftoverBirdsAwarded = true;
            m_LeftoverBirdAwardTimer = 0.0f;
        }

        if (m_PendingLeftoverBirdAwards > 0 && m_BirdLaunchController)
        {
            m_LeftoverBirdAwardTimer += std::max(0.0f, Util::Time::GetDeltaTimeMs() / 1000.0f);
            if (m_LeftoverBirdAwardTimer >= kLeftoverBirdAwardInterval)
            {
                m_LeftoverBirdAwardTimer = 0.0f;
                const int awarded = m_ScoringSystem.AwardLeftoverBirds(1);
                glm::vec2 popupPosition = m_BirdLaunchController->GetBirdAnchorPosition() + glm::vec2{0.0f, 44.0f};
                if (!m_PendingLeftoverBirdAwardPositions.empty())
                {
                    popupPosition = m_PendingLeftoverBirdAwardPositions.front();
                    m_PendingLeftoverBirdAwardPositions.pop_front();
                }
                const glm::vec2 popupOffset = glm::vec2{34.0f, 18.0f};
                SpawnOutlinedFloatingScore(popupPosition + popupOffset,
                                           FormatScore(awarded),
                                           Util::Color::FromRGB(214, 54, 54),
                                           42,
                                           1.65f,
                                           glm::vec2{0.0f, 34.0f});
                --m_PendingLeftoverBirdAwards;
                UpdateScoreHud();
            }
            return;
        }

        m_ScoringSystem.CommitCurrentScoreToHighScore();
        m_HudHighScore = m_ScoringSystem.GetHighScore();
        PersistLevelHighScore();
        m_IsLevelClearScreenVisible = true;
        m_LevelClearAnimationTime = 0.0f;
        SetPauseMenuVisible(false);
    }

    m_LevelClearAnimationTime += std::max(0.0f, Util::Time::GetDeltaTimeMs() / 1000.0f);
    m_GameHud.SetControlsVisible(false);

    const glm::vec2 cameraPos = Util::GetCameraPosition();
    const glm::vec2 viewportSize = Util::GetViewportSize();
    const float zoom = Util::GetCameraZoom();
    const float hudScale = GetHudScale();
    const int score = m_ScoringSystem.GetScore();
    const int stars = m_ScoringSystem.GetStarCount(score);
    const int highScore = m_ScoringSystem.GetHighScore();
    const int highScoreStars = m_ScoringSystem.GetStarCount(highScore);
    m_LevelResultPanel.ShowClear(cameraPos,
                                 viewportSize,
                                 hudScale,
                                 zoom,
                                 score,
                                 stars,
                                 highScore,
                                 highScoreStars,
                                 m_LevelClearAnimationTime);
}

void GameScene::UpdateFailState()
{
    if (!m_LevelFailed)
    {
        const bool hasFailedByExhaustingBirds =
            m_BirdLaunchController &&
            !m_LevelCleared &&
            m_RemainingPigCount > 0 &&
            m_BirdLaunchController->IsOutOfBirds();

        if (!hasFailedByExhaustingBirds)
        {
            return;
        }

        m_LevelFailed = true;
    }

    if (m_IsLevelFailedScreenVisible || m_IsLevelClearScreenVisible)
    {
        return;
    }

    m_IsLevelFailedScreenVisible = true;
    SetPauseMenuVisible(false);
    m_GameHud.SetControlsVisible(false);

    const glm::vec2 cameraPos = Util::GetCameraPosition();
    const glm::vec2 viewportSize = Util::GetViewportSize();
    const float zoom = Util::GetCameraZoom();
    const float hudScale = GetHudScale();
    m_LevelResultPanel.ShowFailed(cameraPos, viewportSize, hudScale, zoom);
}

void GameScene::SpawnOutlinedFloatingScore(const glm::vec2 &position,
                                           const std::string &text,
                                           const Util::Color &frontColor,
                                           const int fontSize,
                                           const float lifeTime,
                                           const glm::vec2 &velocity)
{
    m_FloatingScoreManager.SpawnOutlinedText(
        [this](const std::shared_ptr<Util::GameObject> &element, const float ttl) { AddDebugEntity(element, ttl); },
        position,
        text,
        frontColor,
        fontSize,
        lifeTime,
        velocity);
}

void GameScene::SpawnFloatingScore(const glm::vec2 &position,
                                   const int points,
                                   const Util::Color &frontColor)
{
    m_FloatingScoreManager.SpawnScore(
        [this](const std::shared_ptr<Util::GameObject> &element, const float ttl) { AddDebugEntity(element, ttl); },
        position,
        points,
        frontColor);
}

void GameScene::FinalizeScoreForCharacter(
    const std::shared_ptr<Character> &character, const glm::vec2 &atPosition) {
  if (!character) {
    return;
  }

  if (m_IsLevelClearScreenVisible || m_IsLevelFailedScreenVisible) {
    return;
  }

  int awarded = 0;
  if (character->GetEntityKind() == Character::EntityKind::Pig) {
    awarded = m_ScoringSystem.AwardPigDestroyed();
    if (m_RemainingPigCount > 0) {
      --m_RemainingPigCount;
    }
    if (m_RemainingPigCount == 0) {
      m_LevelCleared = true;
    }
  } else if (IsLevelThreeSmileTarget(m_LevelManager.get(), character)) {
    awarded = m_ScoringSystem.AwardSpecialItemDestroyed();
    if (m_RemainingPigCount > 0) {
      --m_RemainingPigCount;
    }
    if (m_RemainingPigCount == 0) {
      m_LevelCleared = true;
    }
  } else if (character->IsSpecialItem()) {
    awarded = m_ScoringSystem.AwardSpecialItemDestroyed();
  } else if (character->GetEntityKind() == Character::EntityKind::Environment) {
    awarded = m_ScoringSystem.AwardBlockDestroyed(character->GetMaterialType());
  }

  if (awarded > 0) {
    SpawnFloatingScore(atPosition, awarded,
                       Util::Color::FromRGB(255, 255, 255));
  }

  UpdateScoreHud();
}

void GameScene::OnCharacterDeath(const std::shared_ptr<Character> &character) {
  FinalizeScoreForCharacter(character, character ? character->GetPosition()
                                                 : glm::vec2{0.0f, 0.0f});
}

void GameScene::TogglePauseMenu() {
  if (m_IsLevelClearScreenVisible || m_IsLevelFailedScreenVisible) {
    return;
  }

  SetPauseMenuVisible(!m_IsPauseMenuVisible);
}

void GameScene::ToggleMusicMute() {
  m_IsMusicMuted = !m_IsMusicMuted;
  SoundEffect::SetMuted(m_IsMusicMuted);

  if (m_PauseMenu) {
    m_PauseMenu->SetMusicMuted(m_IsMusicMuted);
  }
}
