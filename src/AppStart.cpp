#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "Resource.hpp"

void App::Start()
{
  LOG_TRACE("Start");

  // 初始化 Splash 背景 (靜止圖)
  m_splashBackground = std::make_shared<BackgroundImage>(
      Resource::SPLASH_IMAGE);
  m_splashBackground->SetVisible(true);
  m_splashBackground->SetZIndex(100); // 確保在最上層

  // 初始化移動背景 (動態滾動)
  m_movingBackground = std::make_shared<DynamicBackground>();
  m_movingBackground->SetVisible(false); // 初始先隱藏

  // 紀錄啟動時間
  m_startTime = Util::Time::GetElapsedTimeMs();

  m_bird = std::make_shared<Character>(Resource::BIRD_R);
  m_bird->SetVisible(false);

  m_BGM = std::make_shared<BackgroundMusic>();
  m_BGM->Stop_BGM();

  m_playbutton = std::make_shared<Character>(Resource::BIRD_Y);
  m_playbutton->SetZIndex(50);
  m_playbutton->SetVisible(true);

  m_exitbutton = std::make_shared<Character>(Resource::BIRD_B);
  m_exitbutton->SetZIndex(50);
  m_exitbutton->SetPosition({-1000, -550});
  m_exitbutton->SetVisible(true);

  m_settingbutton = std::make_shared<Character>(Resource::BIRD_B);
  m_settingbutton->SetZIndex(50);
  m_settingbutton->SetPosition({1000, -550});
  m_settingbutton->SetVisible(true);

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

void App::End()
{ // NOLINT(this method will mutate members in the future)
  LOG_TRACE("End");
}
