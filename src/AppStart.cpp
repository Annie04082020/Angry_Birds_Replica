#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Logger.hpp"

void App::Start() {
  LOG_TRACE("Start");
  m_bird = std::make_shared<Character>(RESOURCE_DIR "/temp/R.png");
  m_bird->SetVisible(false);
  m_background = std::make_shared<BackgroundImage>();
  m_BGM = std::make_shared<BackgroundMusic>();
  m_BGM->Play_BGM();

  m_Root.AddChild(m_background);
  m_Root.AddChild(m_bird);
  m_Root.AddChild(m_BGM);

  m_CurrentState = State::UPDATE;
}

void App::End() { // NOLINT(this method will mutate members in the future)
  LOG_TRACE("End");
}
