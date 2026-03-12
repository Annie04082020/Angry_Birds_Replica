#ifndef APP_HPP
#define APP_HPP

#include "BGM.hpp"
#include "BackgroundImage.hpp"
#include "Character.hpp"
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
  std::shared_ptr<BackgroundImage> m_background;
  std::shared_ptr<BackgroundMusic> m_BGM;
  Util::Renderer m_Root;
};

#endif
