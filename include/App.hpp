#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export
#include "Character.hpp"
#include "BackgroundImage.hpp"
#include "Util/Renderer.hpp"


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
    //Put things you need here
    State m_CurrentState = State::START;
    std::shared_ptr<Character> m_bird;
    std::shared_ptr<BackgroundImage> m_background;
    Util::Renderer m_Root;


};

#endif
