#ifndef INTRO_SCENE_HPP
#define INTRO_SCENE_HPP

#include "Button.hpp"
#include "Character.hpp"
#include "DynamicBackground.hpp"
#include "Scene.hpp"
#include <memory>

class IntroScene : public Scene
{
public:
    static std::shared_ptr<IntroScene> Create();
    void Update() override;

protected:
    IntroScene(std::shared_ptr<DynamicBackground> bg);

private:
    std::shared_ptr<DynamicBackground> m_movingBg;
    std::shared_ptr<Character> m_bird;
    std::shared_ptr<Button> m_playbutton;
    std::shared_ptr<Button> m_exitbutton;
    std::shared_ptr<Button> m_settingbutton;
};

#endif // INTRO_SCENE_HPP
