#include "App.hpp"

#include "Resource.hpp"
#include "Util/Image.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

void App::Start() {
  LOG_TRACE("Start");

  // 初始化 Splash 背景 (靜止圖)
  m_splashBackground =
      std::make_shared<BackgroundImage>(Resource::SPLASH_IMAGE);
  m_splashBackground->SetVisible(true);
  m_splashBackground->SetZIndex(100); // 確保在最上層

  // 初始化移動背景 (動態滾動)
  m_movingBackground = std::make_shared<DynamicBackground>();
  m_movingBackground->SetVisible(false); // 初始先隱藏

  // 紀錄啟動時間
  m_startTime = Util::Time::GetElapsedTimeMs();

  m_bird = std::make_shared<Character>(Resource::BIRD_R);
  m_bird->SetVisible(false);

  m_BGM = std::make_shared<BackgroundMusic>(Resource::TITLE_THEME);
  m_BGM->Stop_BGM();

  m_playbutton = std::make_shared<Button>(Resource::Play_Button);
  m_playbutton->SetZIndex(50);
  m_playbutton->SetVisible(true);
  m_playbutton->SetOnClickFunction([this]() {
    m_playbutton->SetVisible(false);
    m_bird->SetVisible(true);
  });

  m_exitbutton = std::make_shared<Button>(Resource::Exit_Button);
  m_exitbutton->SetZIndex(50);
  m_exitbutton->SetPosition({-1000, -550});
  m_exitbutton->SetVisible(true);
  m_exitbutton->SetOnClickFunction([this]() {
    m_exitbutton->SetVisible(false);
    m_bird->SetVisible(true);
  });

  m_settingbutton = std::make_shared<Button>(Resource::Setting_Button);
  m_settingbutton->SetZIndex(50);
  m_settingbutton->SetPosition({1000, -550});
  m_settingbutton->SetVisible(true);
  m_settingbutton->SetSFX(Resource::SETTING_SFX);
  m_settingbutton->SetOnClickFunction([this]() { m_bird->SetVisible(true); });

  // 將兩個背景都加入 Renderer，順序決定渲染層級
  m_Root.AddChild(m_splashBackground);
  m_Root.AddChild(m_movingBackground);
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
