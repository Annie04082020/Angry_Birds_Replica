#include "App.hpp"
#include "GameScene.hpp"
#include "IntroScene.hpp"
#include "LevelSelectScene.hpp"
#include "Resource.hpp"
#include "Scene.hpp"
#include "Util/Image.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "SDL.h"
#include "SDL_image.h"
#include "config.hpp"
#include <memory>

namespace
{
  std::string ResolveLevelPath(const int levelNumber)
  {
    switch (levelNumber)
    {
    case 1:
      return Resource::LEVEL_1_DATA;
    case 2:
      return Resource::LEVEL_2_DATA;
    case 9:
      return Resource::LEVEL_9_DATA;
    default:
      return "";
    }
  }

  void ApplyStartupHandCursor()
  {
    static SDL_Cursor *defaultHandCursor = nullptr;
    if (defaultHandCursor == nullptr)
    {
      SDL_Surface *surface = IMG_Load(RESOURCE_DIR "/Image/hand/sprite_002.png");
      if (surface != nullptr)
      {
        defaultHandCursor = SDL_CreateColorCursor(surface, 8, 8);
        SDL_FreeSurface(surface);
      }
    }

    if (defaultHandCursor != nullptr)
    {
      SDL_SetCursor(defaultHandCursor);
      SDL_ShowCursor(SDL_ENABLE);
    }
  }
}

void App::Start()
{
  LOG_TRACE("Start");

  ApplyStartupHandCursor();

  m_loadingScene = std::make_shared<Scene>(
      std::make_shared<BackgroundImage>(Resource::SPLASH_IMAGE));
  m_loadingScene->SetZIndex(100);
  m_loadingScene->SetVisible(true);

  m_introScene = IntroScene::Create();
  m_levelSelectScene = LevelSelectScene::Create();
  m_introScene->SetOnPlayClickCallback([this]()
                                       {
                                         this->ShowLevelSelectScene();
                                         return true;
                                       });
  m_levelSelectScene->SetOnBackClickCallback([this]()
                                             { this->ShowIntroScene(); });
  m_levelSelectScene->SetOnLevelSelectCallback([this](const int levelNumber)
                                         { return this->TransitionToGame(levelNumber); });

  // 紀錄啟動時間
  m_startTime = Util::Time::GetElapsedTimeMs();

  m_Root.AddChild(m_loadingScene);
  m_Root.AddChild(m_introScene);
  m_Root.AddChild(m_levelSelectScene);

  ShowIntroScene();

  m_CurrentState = State::UPDATE;
}

void App::ShowIntroScene()
{
  if (m_introScene)
  {
    m_introScene->SetVisible(true);
    m_introScene->SetMenuVisible(true);
  }
  if (m_levelSelectScene)
  {
    m_levelSelectScene->SetVisible(false);
    m_levelSelectScene->SetSceneVisible(false);
  }
}

void App::ShowLevelSelectScene()
{
  if (m_introScene)
  {
    m_introScene->SetVisible(false);
    m_introScene->SetMenuVisible(false);
  }
  if (m_levelSelectScene)
  {
    m_levelSelectScene->SetVisible(true);
    m_levelSelectScene->SetSceneVisible(true);
  }
}

bool App::TransitionToGame(const int levelNumber)
{
  if (m_CurrentState == State::GAME)
  {
    return true;
  }
  LOG_DEBUG("Transitioning to GAME state");

  const std::string levelPath = ResolveLevelPath(levelNumber);
  if (levelPath.empty())
  {
    LOG_WARN("Level %d is not implemented yet", levelNumber);
    return false;
  }

  if (!LoadLevel(levelPath))
  {
    LOG_ERROR("Failed to load level %d", levelNumber);
    return false;
  }

  return true;
}

bool App::LoadLevel(const std::string &levelPath)
{
  // Unload any existing game scene first
  UnloadCurrentGameScene();

  m_gameScene = std::make_shared<GameScene>(
      std::make_shared<DynamicBackground>(Resource::MOVING_BG_IMAGE));

  if (m_gameScene && m_gameScene->LoadLevel(levelPath))
  {
    LOG_DEBUG("Level loaded successfully: %s", levelPath.c_str());
    m_loadingScene->SetVisible(false);
    if (m_introScene)
    {
      m_introScene->SetVisible(false);
      m_introScene->SetMenuVisible(false);
    }
    if (m_levelSelectScene)
    {
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

void App::UnloadCurrentGameScene()
{
  if (m_gameScene)
  {
    m_Root.RemoveChild(m_gameScene);
    m_gameScene.reset();
  }
}

void App::End()
{ // NOLINT(this method will mutate members in the future)
  LOG_TRACE("End");

  if (m_gameScene)
  {
    m_Root.RemoveChild(m_gameScene);
    m_gameScene.reset();
  }

  if (m_introScene)
  {
    m_Root.RemoveChild(m_introScene);
    m_introScene.reset();
  }

  if (m_levelSelectScene)
  {
    m_Root.RemoveChild(m_levelSelectScene);
    m_levelSelectScene.reset();
  }

  if (m_loadingScene)
  {
    m_Root.RemoveChild(m_loadingScene);
    m_loadingScene.reset();
  }
}
