#include "App.hpp"
#include "GameScene.hpp"
#include "IntroScene.hpp"
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
  m_introScene->SetOnPlayClickCallback([this]()
                                       { this->TransitionToGame(); });

  // 紀錄啟動時間
  m_startTime = Util::Time::GetElapsedTimeMs();

  m_Root.AddChild(m_loadingScene);
  m_Root.AddChild(m_introScene);

  m_CurrentState = State::UPDATE;
}

void App::TransitionToGame()
{
  if (m_CurrentState == State::GAME)
  {
    return;
  }
  LOG_DEBUG("Transitioning to GAME state");

  if (!LoadLevel(Resource::LEVEL_1_DATA))
  {
    LOG_ERROR("Failed to load level 1");
  }
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
      m_introScene->SetVisible(false);

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
}
