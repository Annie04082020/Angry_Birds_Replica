#include "App.hpp"
#include "Scene.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Time.hpp"

void App::Update()
{
  if (Util::Input::IsKeyPressed(Util::Keycode::F10))
  {
    ToggleLevelSelectCheatMode();
  }

  if (!m_isSplashDone)
  {
    if (Util::Time::GetElapsedTimeMs() - m_startTime >= 2000.0f)
    {
      m_loadingScene->SetVisible(false);
      if (m_introScene)
      {
        m_introScene->SetVisible(true);
        m_introScene->Init();
      }
      if (m_levelSelectScene)
      {
        m_levelSelectScene->Init();
      }
      m_isSplashDone = true;
    }
  }

  if (m_isSplashDone && m_CurrentState != State::GAME)
  {
    if (m_introScene)
    {
      m_introScene->Update();
    }
    if (m_levelSelectScene)
    {
      m_levelSelectScene->Update();
    }
  }

  if (m_CurrentState == State::GAME && m_gameScene)
  {
    m_gameScene->Update();
  }

  if (m_pendingGameAction != PendingGameAction::None)
  {
    const PendingGameAction action = m_pendingGameAction;
    m_pendingGameAction = PendingGameAction::None;

    if (action == PendingGameAction::RestartCurrentLevel)
    {
      RestartCurrentLevel();
    }
    else if (action == PendingGameAction::OpenLevelSelect)
    {
      UnloadCurrentGameScene();
      ShowLevelSelectScene();
      m_CurrentState = State::UPDATE;
    }
    else if (action == PendingGameAction::OpenNextLevel)
    {
      OpenNextLevel();
    }
  }

  m_Root.Update();

  if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit())
  {
    m_CurrentState = State::END;
  }
}
