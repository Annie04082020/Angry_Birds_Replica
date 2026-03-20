#include "App.hpp"
#include "Resource.hpp"
#include "Scene.hpp"
#include "Util/Image.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include <memory>

void App::Start() {
  LOG_TRACE("Start");

  m_loadingScene = std::make_shared<Scene>(
      std::make_shared<BackgroundImage>(Resource::SPLASH_IMAGE));
  m_loadingScene->SetZIndex(100);
  m_loadingScene->SetVisible(true);

  // 初始化移動背景 (動態滾動)
  auto movingBg =
      std::make_shared<DynamicBackground>(Resource::MOVING_BG_IMAGE);
  m_introScene = std::make_shared<Scene>(movingBg);
  m_introScene->SetVisible(false); // 初始先隱藏
  m_introScene->SetZIndex(50);
  m_introScene->SetOnUpdate([movingBg]() { movingBg->Update(); });
  m_bird = std::make_shared<Character>(Resource::BIRD_R);
  m_bird->SetVisible(false);
  m_introScene->AddElements(m_bird);
  m_introScene->SetBGM(
      std::make_shared<BackgroundMusic>(Resource::TITLE_THEME));

  // 紀錄啟動時間
  m_startTime = Util::Time::GetElapsedTimeMs();
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

  m_Root.AddChild(m_loadingScene);
  m_Root.AddChild(m_introScene);
  m_Root.AddChild(m_bird);
  m_Root.AddChild(m_exitbutton);
  m_Root.AddChild(m_settingbutton);
  m_Root.AddChild(m_playbutton);

  m_CurrentState = State::UPDATE;
}

void App::End() { // NOLINT(this method will mutate members in the future)
  LOG_TRACE("End");
}
