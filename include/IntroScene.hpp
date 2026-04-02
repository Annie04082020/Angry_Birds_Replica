#ifndef INTRO_SCENE_HPP
#define INTRO_SCENE_HPP

#include "Button.hpp"
#include "Character.hpp"
#include "DynamicBackground.hpp"
#include "Scene.hpp"
#include <functional>
#include <memory>

class IntroScene : public Scene
{
public:
    static std::shared_ptr<IntroScene> Create();
    void Update() override;
    
    void SetOnPlayClickCallback(std::function<void()> callback) {
        m_onPlayClick = callback;
    }

protected:
    IntroScene(std::shared_ptr<DynamicBackground> bg);

private:
    std::shared_ptr<DynamicBackground> m_movingBg;
    std::shared_ptr<Character> m_bird;
    std::shared_ptr<Button> m_playbutton;
    std::shared_ptr<Button> m_exitbutton;
    std::shared_ptr<Button> m_settingbutton;
    std::shared_ptr<Util::GameObject> m_settingOverlay;
    std::function<void()> m_onPlayClick = nullptr;
    
    float m_overlayRotation = 0.0f;
    bool m_isRotating = false;
    float m_rotationSpeed = 10.0f; // degrees per frame
    float m_targetRotation = 0.0f;
    
};

#endif // INTRO_SCENE_HPP
