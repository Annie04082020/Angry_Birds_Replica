#include "App.hpp"
#include "GameScene.hpp"
#include "IntroScene.hpp"
#include "Resource.hpp"
#include "Scene.hpp"
#include "Util/Image.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include <memory>

void App::Start()
{
  LOG_TRACE("Start");

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

  m_gameScene = std::make_shared<GameScene>(
      std::make_shared<DynamicBackground>(Resource::MOVING_BG_IMAGE));

  // Load level 1 through GameScene
  if (m_gameScene && m_gameScene->LoadLevel(Resource::LEVEL_1_DATA))
  {
    LOG_DEBUG("Level 1 loaded successfully");
    m_loadingScene->SetVisible(false);
    m_introScene->SetVisible(false);

    m_Root.AddChild(m_gameScene);
    m_gameScene->Init();

    m_CurrentState = State::GAME;
  }
  else
  {
    LOG_ERROR("Failed to load level 1");
  }
}

void App::End()
{ // NOLINT(this method will mutate members in the future)
  LOG_TRACE("End");
}
