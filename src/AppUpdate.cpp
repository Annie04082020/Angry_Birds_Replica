#include "App.hpp"
#include "Scene.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Time.hpp"

void App::Update()
{
  // 使用絕對時間差來判斷是否過了 2 秒
  if (!m_isSplashDone)
  {
    if (Util::Time::GetElapsedTimeMs() - m_startTime >= 2000.0f)
    {
      m_loadingScene->SetVisible(false);
      m_introScene->SetVisible(true);
      m_introScene->Init();
      m_isSplashDone = true;
    }
  }

  // 只有在 Splash 結束且尚未進入 GAME 時更新 intro scene
  if (m_isSplashDone && m_CurrentState != State::GAME)
  {
    m_introScene->Update();
  }

  if (m_CurrentState == State::GAME && m_gameScene)
  {
    m_gameScene->Update();
  }

  // 按鈕的互動邏輯現在已由 m_introScene 內部自動管理！

  m_Root.Update();

  /*
   * Do not touch the code below as they serve the purpose for
   * closing the window.
   */
  if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit())
  {
    m_CurrentState = State::END;
  }
}
