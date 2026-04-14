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

    void SetOnPlayClickCallback(std::function<void()> callback)
    {
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
    std::shared_ptr<Button> m_additionalButton;
    std::shared_ptr<Util::GameObject> m_additionalButtonOverlay;
    std::shared_ptr<Util::GameObject> m_menuItem017;
    std::shared_ptr<Util::GameObject> m_menuItem032;
    std::shared_ptr<Util::GameObject> m_menuItem043;
    std::shared_ptr<Util::GameObject> m_additionalMenuItem108;
    std::shared_ptr<Util::GameObject> m_additionalMenuItem006;
    std::shared_ptr<Util::GameObject> m_additionalMenuItem041;
    std::shared_ptr<Util::GameObject> m_exitConfirm048;
    std::shared_ptr<Util::GameObject> m_exitButton105;
    std::shared_ptr<Util::GameObject> m_exitButton95;
    std::shared_ptr<Util::GameObject> m_exitDialog;
    glm::vec2 m_settingScale = {0.9f, 0.9f};
    glm::vec2 m_settingScaleHover = {1.0f, 1.0f};
    float m_settingOverlayTargetRotation = 0.0f;
    bool m_settingOverlayIsAnimating = false;
    glm::vec2 m_additionalScale = {0.9f, 0.9f};
    glm::vec2 m_additionalScaleHover = {1.0f, 1.0f};
    float m_additionalOverlayTargetRotation = 0.0f;
    bool m_additionalOverlayIsAnimating = false;
    bool m_settingMenuOpen = false;
    bool m_additionalMenuOpen = false;
    bool m_menuItemsAnimating = false;
    bool m_additionalMenuItemsAnimating = false;
    bool m_exitPanelVisible = false;
    glm::vec2 m_settingButtonPosition = {520.0f, -300.0f};
    glm::vec2 m_additionalButtonPosition = {650.0f, -300.0f};

    // Menu configuration from JSON
    float m_menuItemSpacing = 70.0f;
    float m_menuInitialOffset = -5.0f;
    float m_menuAnimationDistance = 300.0f;
    float m_menuAnimationSpeed = 400.0f;

    std::function<void()> m_onPlayClick = nullptr;
};

#endif // INTRO_SCENE_HPP
