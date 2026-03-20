#ifndef APP_HPP
#define APP_HPP

#include "BGM.hpp"
#include "Button.hpp"
#include "Character.hpp"
#include "DynamicBackground.hpp"
#include "Scene.hpp"
#include "SoundEffect.hpp"
#include "Util/Renderer.hpp"
#include "pch.hpp" // IWYU pragma: export

class App {
public:
  enum class State {
    START,
    UPDATE,
    END,
  };

  State GetCurrentState() const { return m_CurrentState; }

  void Start();

  void Update();

  void End(); // NOLINT(readability-convert-member-functions-to-static)

private:
  void ValidTask();

private:
  // Put things you need here
  State m_CurrentState = State::START;
  std::shared_ptr<Character> m_bird;
  std::shared_ptr<Button> m_playbutton;
  std::shared_ptr<Button> m_exitbutton;
  std::shared_ptr<Button> m_settingbutton;
  std::shared_ptr<BackgroundImage> m_splashBackground;
  std::shared_ptr<DynamicBackground> m_movingBackground;
  std::shared_ptr<BackgroundMusic> m_BGM;
  std::shared_ptr<SoundEffect> m_SFX;
  std::shared_ptr<Scene> m_loadingScene;
  std::shared_ptr<Scene> m_introScene;
  float m_startTime = 0.0f;
  bool m_isSplashDone = false;
  Util::Renderer m_Root;
};

#endif
