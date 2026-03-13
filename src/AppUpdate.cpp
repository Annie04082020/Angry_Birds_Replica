#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

void App::Update() {

  // Play button interaction logic
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
