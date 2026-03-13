#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Time.hpp"

void App::Update() {
  // 使用絕對時間差來判斷是否過了 2 秒
  if (!m_isSplashDone) {
    if (Util::Time::GetElapsedTimeMs() - m_startTime >= 2000.0f) {
      m_splashBackground->SetVisible(false);
      m_movingBackground->SetVisible(true);
      m_BGM->Play_BGM();
      m_isSplashDone = true;
    }
  }

  // 只有在 Splash 結束後或是需要時才更新移動背景
  if (m_isSplashDone) {
    m_movingBackground->Update();
  }

  // 按鈕互動邏輯
  if (m_CurrentState == State::UPDATE) {
    auto mousePos = Util::Input::GetCursorPosition();

    m_playbutton->UpdateHoverScale(mousePos);
    if (m_playbutton->IfClicked(mousePos)) {
      m_playbutton->SetVisible(false);
      m_bird->SetVisible(true);
    }

    m_settingbutton->UpdateHoverScale(mousePos);
    if (m_settingbutton->IfClicked(mousePos)) {
      m_settingbutton->SetVisible(false);
      m_bird->SetVisible(true);
    }

    m_exitbutton->UpdateHoverScale(mousePos);
    if (m_exitbutton->IfClicked(mousePos)) {
      m_exitbutton->SetVisible(false);
      m_bird->SetVisible(true);
    }
  }

  m_Root.Update();
  /*
   * Do not touch the code below as they serve the purpose for
   * closing the window.
   */
  if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
    m_CurrentState = State::END;
  }
}
