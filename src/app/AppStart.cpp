#include "App.hpp"
#include "GameScene.hpp"
#include "IntroScene.hpp"
#include "LevelSelectScene.hpp"
#include "JsonParseUtils.hpp"
#include "Resource.hpp"
#include "SDL.h"
#include "SDL_image.h"
#include "Scene.hpp"
#include "Util/Image.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "Util/TransformUtils.hpp"
#include "config.hpp"
#include <fstream>
#include <memory>
#include <sstream>

namespace {
constexpr const char *kHighScoreFilePath = RESOURCE_DIR "/high_scores.json";
constexpr const char *kUnlockProgressFilePath =
    RESOURCE_DIR "/unlock_progress.json";
constexpr int kTotalLevels = 10;

std::string ResolveLevelPath(const int levelNumber) {
  switch (levelNumber) {
  case 1:
    return Resource::LEVEL_1_DATA;
  case 2:
    return Resource::LEVEL_2_DATA;
  case 3:
    return Resource::LEVEL_3_DATA;
  case 4:
    return Resource::LEVEL_4_DATA;
  case 5:
    return Resource::LEVEL_5_DATA;
  case 6:
    return Resource::LEVEL_6_DATA;
  case 7:
    return Resource::LEVEL_7_DATA;
  case 8:
    return Resource::LEVEL_8_DATA;
  case 9:
    return Resource::LEVEL_9_DATA;
  case 10:
    return Resource::LEVEL_10_DATA;
  default:
    return "";
  }
}

void ApplyStartupHandCursor() {
  static SDL_Cursor *defaultHandCursor = nullptr;
  if (defaultHandCursor == nullptr) {
    SDL_Surface *surface = IMG_Load(RESOURCE_DIR "/Image/hand/sprite_002.png");
    if (surface != nullptr) {
      defaultHandCursor = SDL_CreateColorCursor(surface, 8, 8);
      SDL_FreeSurface(surface);
    }
  }

  if (defaultHandCursor != nullptr) {
    SDL_SetCursor(defaultHandCursor);
    SDL_ShowCursor(SDL_ENABLE);
  }
}

int LoadHighestSequentialClearedLevelFromFile() {
  std::ifstream file(kUnlockProgressFilePath);
  if (!file.is_open()) {
    return 0;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  const std::string jsonContent = buffer.str();
  return std::clamp(
      JsonParseUtils::ExtractInt(jsonContent, "highest_sequential_cleared_level", 0),
      0, kTotalLevels);
}
} // namespace

void App::Start() {
  LOG_TRACE("Start");

  ApplyStartupHandCursor();

  m_loadingScene = std::make_shared<Scene>(
      std::make_shared<BackgroundImage>(Resource::SPLASH_IMAGE));
  m_loadingScene->SetZIndex(100);
  m_loadingScene->SetVisible(true);

  m_introScene = IntroScene::Create();
  m_levelSelectScene = LevelSelectScene::Create();
  m_introScene->SetOnPlayClickCallback([this]() {
    this->ShowLevelSelectScene();
    return true;
  });
  m_levelSelectScene->SetOnBackClickCallback(
      [this]() { this->ShowIntroScene(); });
  m_levelSelectScene->SetOnLevelSelectCallback([this](const int levelNumber) {
    return this->TransitionToGame(levelNumber);
  });

  RefreshLevelSelectProgress();
  ApplyLevelSelectProgress();

  // 紀錄啟動時間
  m_startTime = Util::Time::GetElapsedTimeMs();

  m_Root.AddChild(m_loadingScene);
  m_Root.AddChild(m_introScene);
  m_Root.AddChild(m_levelSelectScene);

  ShowIntroScene();

  m_CurrentState = State::UPDATE;
}

void App::ShowIntroScene() {
  m_currentLevelStartedFromCheatMode = false;
  if (m_introScene) {
    m_introScene->SetVisible(true);
    m_introScene->SetMenuVisible(true);
  }
  if (m_levelSelectScene) {
    m_levelSelectScene->SetVisible(false);
    m_levelSelectScene->SetSceneVisible(false);
  }
  Util::SetCameraZoom(1.0f);
  Util::SetCameraPosition({0.0f, 0.0f});
}

void App::ShowLevelSelectScene() {
  m_currentLevelStartedFromCheatMode = false;
  RefreshLevelSelectProgress();
  ApplyLevelSelectProgress();

  if (m_introScene) {
    m_introScene->SetVisible(false);
    m_introScene->SetMenuVisible(false);
  }
  if (m_levelSelectScene) {
    m_levelSelectScene->SetVisible(true);
    m_levelSelectScene->SetSceneVisible(true);
  }
  Util::SetCameraZoom(1.0f);
  Util::SetCameraPosition({0.0f, 0.0f});
}

void App::RefreshLevelSelectProgress() {
  m_highestSequentialClearedLevel = LoadHighestSequentialClearedLevelFromFile();
}

void App::ApplyLevelSelectProgress() {
  if (m_levelSelectScene) {
    m_levelSelectScene->SetLevelProgress(m_highestSequentialClearedLevel,
                                         m_isLevelSelectCheatMode);
  }
}

void App::ToggleLevelSelectCheatMode() {
  m_isLevelSelectCheatMode = !m_isLevelSelectCheatMode;
  ApplyLevelSelectProgress();
  LOG_INFO("Level select cheat mode {}", m_isLevelSelectCheatMode ? "enabled"
                                                                  : "disabled");
}

void App::SaveUnlockedProgress(const int clearedLevel) const {
  if (clearedLevel <= 0 || clearedLevel > kTotalLevels) {
    return;
  }

  if (m_currentLevelStartedFromCheatMode) {
    LOG_INFO("Skip saving unlock progress for level {} because it was started "
             "from cheat mode",
             clearedLevel);
    return;
  }

  int existingHighest = 0;
  std::ifstream inputFile(kUnlockProgressFilePath);
  if (inputFile.is_open()) {
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    existingHighest = JsonParseUtils::ExtractInt(
        buffer.str(), "highest_sequential_cleared_level", 0);
  }

  const int newHighest = std::max(existingHighest, clearedLevel);

  std::ofstream outputFile(kUnlockProgressFilePath, std::ios::trunc);
  if (!outputFile.is_open()) {
    LOG_WARN("Failed to save unlock progress file: {}", kUnlockProgressFilePath);
    return;
  }

  outputFile << "{\n";
  outputFile << "  \"highest_sequential_cleared_level\": " << newHighest << "\n";
  outputFile << "}\n";
}

bool App::TransitionToGame(const int levelNumber) {
  LOG_DEBUG("Transitioning to GAME state");

  RefreshLevelSelectProgress();
  const int highestUnlockedLevel =
      std::clamp(m_highestSequentialClearedLevel + 1, 1, kTotalLevels);
  if (!m_isLevelSelectCheatMode && levelNumber > highestUnlockedLevel) {
    LOG_WARN("Level {} is locked. Highest unlocked level is {}", levelNumber,
             highestUnlockedLevel);
    ApplyLevelSelectProgress();
    return false;
  }

  const std::string levelPath = ResolveLevelPath(levelNumber);
  if (levelPath.empty()) {
    LOG_WARN("Level {} is not implemented yet", levelNumber);
    return false;
  }

  m_currentLevelNumber = levelNumber;
  m_currentLevelPath = levelPath;
  m_currentLevelStartedFromCheatMode = m_isLevelSelectCheatMode;

  if (!LoadLevel(levelPath)) {
    LOG_ERROR("Failed to load level {}", levelNumber);
    m_currentLevelStartedFromCheatMode = false;
    return false;
  }

  return true;
}

bool App::LoadLevel(const std::string &levelPath) {
  // Unload any existing game scene first
  UnloadCurrentGameScene();

  m_gameScene = std::make_shared<GameScene>(
      std::make_shared<DynamicBackground>(Resource::MOVING_BG_IMAGE));

  m_gameScene->SetOnRestartLevelCallback([this]() {
    m_pendingGameAction = PendingGameAction::RestartCurrentLevel;
  });
  m_gameScene->SetOnOpenLevelSelectCallback(
      [this]() { m_pendingGameAction = PendingGameAction::OpenLevelSelect; });
  m_gameScene->SetOnNextLevelCallback(
      [this]() { m_pendingGameAction = PendingGameAction::OpenNextLevel; });
  m_gameScene->SetOnLevelClearedCallback(
      [this](const int clearedLevel) { SaveUnlockedProgress(clearedLevel); });

  if (m_gameScene && m_gameScene->LoadLevel(levelPath)) {
    LOG_DEBUG("Level loaded successfully: {}", levelPath);
    m_loadingScene->SetVisible(false);
    if (m_introScene) {
      m_introScene->SetVisible(false);
      m_introScene->SetMenuVisible(false);
    }
    if (m_levelSelectScene) {
      m_levelSelectScene->SetVisible(false);
      m_levelSelectScene->SetSceneVisible(false);
    }

    m_Root.AddChild(m_gameScene);
    m_gameScene->Init();

    m_CurrentState = State::GAME;
    return true;
  }

  // cleanup on failure
  m_gameScene.reset();
  return false;
}

bool App::RestartCurrentLevel() {
  if (m_currentLevelNumber > 0) {
    const std::string levelPath = ResolveLevelPath(m_currentLevelNumber);
    if (!levelPath.empty()) {
      m_currentLevelPath = levelPath;
      return LoadLevel(levelPath);
    }
  }

  if (m_currentLevelPath.empty()) {
    return false;
  }

  return LoadLevel(m_currentLevelPath);
}

bool App::OpenNextLevel() {
  if (m_currentLevelNumber <= 0) {
    return false;
  }

  const int nextLevelNumber = m_currentLevelNumber + 1;
  const std::string nextLevelPath = ResolveLevelPath(nextLevelNumber);
  if (nextLevelPath.empty()) {
    UnloadCurrentGameScene();
    ShowLevelSelectScene();
    m_CurrentState = State::UPDATE;
    return false;
  }

  m_currentLevelNumber = nextLevelNumber;
  m_currentLevelPath = nextLevelPath;
  return LoadLevel(nextLevelPath);
}

void App::UnloadCurrentGameScene() {
  if (m_gameScene) {
    m_Root.RemoveChild(m_gameScene);
    m_gameScene.reset();
  }
}

void App::End() { // NOLINT(this method will mutate members in the future)
  LOG_TRACE("End");

  if (m_gameScene) {
    m_Root.RemoveChild(m_gameScene);
    m_gameScene.reset();
  }

  if (m_introScene) {
    m_Root.RemoveChild(m_introScene);
    m_introScene.reset();
  }

  if (m_levelSelectScene) {
    m_Root.RemoveChild(m_levelSelectScene);
    m_levelSelectScene.reset();
  }

  if (m_loadingScene) {
    m_Root.RemoveChild(m_loadingScene);
    m_loadingScene.reset();
  }
}
