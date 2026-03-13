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
  m_playbutton = std::make_shared<Character>(RESOURCE_DIR "/temp/Y.png");
  m_playbutton->SetZIndex(50); // Ensure it renders on top of everything
  m_playbutton->SetVisible(true);
  m_exitbutton = std::make_shared<Character>(RESOURCE_DIR "/temp/B.png");
  m_exitbutton->SetZIndex(50); // Ensure it renders on top of everything
  m_exitbutton->SetPosition({-1000, -550});
  m_exitbutton->SetVisible(true);
  m_settingbutton = std::make_shared<Character>(RESOURCE_DIR "/temp/B.png");
  m_settingbutton->SetZIndex(50); // Ensure it renders on top of everything
  m_settingbutton->SetPosition({1000, -550});
  m_settingbutton->SetVisible(true);

  m_Root.AddChild(m_background);
  m_Root.AddChild(m_bird);
  m_Root.AddChild(m_exitbutton);
  m_Root.AddChild(m_settingbutton);
  m_Root.AddChild(m_BGM);
  m_Root.AddChild(m_playbutton);

  m_CurrentState = State::UPDATE;
}

void App::End() { // NOLINT(this method will mutate members in the future)
  LOG_TRACE("End");
}
