#include "App.hpp"
#include "IntroScene.hpp"
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

  m_introScene = IntroScene::Create();

  // 紀錄啟動時間
  m_startTime = Util::Time::GetElapsedTimeMs();

  m_Root.AddChild(m_loadingScene);
  m_Root.AddChild(m_introScene);

  m_CurrentState = State::UPDATE;
}

void App::End() { // NOLINT(this method will mutate members in the future)
  LOG_TRACE("End");
}
