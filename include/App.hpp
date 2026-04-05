#ifndef APP_HPP
#define APP_HPP

#include "BGM.hpp"
#include "Button.hpp"
#include "Character.hpp"
#include "DynamicBackground.hpp"
#include "GameScene.hpp"
#include "IntroScene.hpp"
#include "LevelManager.hpp"
#include "Scene.hpp"
#include "SoundEffect.hpp"
#include "Util/Renderer.hpp"
#include "pch.hpp" // IWYU pragma: export

class App
{
public:
  enum class State
  {
    START,
    UPDATE,
    GAME,
    END,
  };

  State GetCurrentState() const { return m_CurrentState; }

  void Start();

  void Update();

  void End(); // NOLINT(readability-convert-member-functions-to-static)

  void TransitionToGame();
  bool LoadLevel(const std::string &levelPath);
  void UnloadCurrentGameScene();

private:
  void ValidTask();

private:
  // Put things you need here
  State m_CurrentState = State::START;
  std::shared_ptr<Scene> m_loadingScene;
  std::shared_ptr<IntroScene> m_introScene;
  std::shared_ptr<GameScene> m_gameScene;
  float m_startTime = 0.0f;
  bool m_isSplashDone = false;
  Util::Renderer m_Root;
};

#endif
