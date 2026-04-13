#include "IntroScene.hpp"
#include "BGM.hpp"
#include "Resource.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include <cmath>

std::shared_ptr<IntroScene> IntroScene::Create()
{
    auto bg = std::make_shared<DynamicBackground>(Resource::MOVING_BG_IMAGE);
    struct Enabler : public IntroScene
    {
        Enabler(std::shared_ptr<DynamicBackground> bg) : IntroScene(bg) {}
    };
    return std::make_shared<Enabler>(bg);
}

IntroScene::IntroScene(std::shared_ptr<DynamicBackground> bg)
    : Scene(bg), m_movingBg(bg)
{
    SetVisible(true);
    SetZIndex(50);
    SetBGM(std::make_shared<BackgroundMusic>(Resource::TITLE_THEME));

    m_bird = std::make_shared<Character>(Resource::BIRD_R);
    m_bird->SetVisible(false);

    m_playbutton = std::make_shared<Button>(Resource::Play_Button);
    m_playbutton->SetZIndex(50);
    m_playbutton->SetVisible(true);
    m_playbutton->SetOnClickFunction([this]()
                                     {
        m_playbutton->SetVisible(false);
        m_exitbutton->SetVisible(false);
        m_settingbutton->SetVisible(false);
        m_settingOverlay->SetVisible(false);
        m_bird->SetVisible(true);
        if (m_onPlayClick) {
            m_onPlayClick();
        }
    });

    m_exitbutton = std::make_shared<Button>(Resource::Exit_Button);
    m_exitbutton->SetZIndex(50);
    m_exitbutton->SetPosition({-1000, -550});
    m_exitbutton->SetVisible(true);
    m_exitbutton->SetOnClickFunction([this]()
                                     {
        m_exitbutton->SetVisible(false);
        m_bird->SetVisible(true); });

    constexpr glm::vec2 settingPosition = {520.0f, -300.0f};
    constexpr glm::vec2 settingScale = {0.8f, 0.8f};

    m_settingbutton = std::make_shared<Button>(Resource::Setting_Button_Base);
    m_settingbutton->SetZIndex(50);
    m_settingbutton->SetPosition(settingPosition);
    m_settingbutton->SetScale(settingScale);
    m_settingbutton->SetVisible(true);
    m_settingbutton->SetSFX(Resource::SETTING_SFX);
    m_settingbutton->SetOnClickFunction([this]() {
        m_bird->SetVisible(true);
        m_settingOverlayTargetRotation -= 3.14159265f;
        m_settingOverlayIsAnimating = true;
    });

    m_settingOverlay = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Button_Overlay), 51);
    m_settingOverlay->m_Transform.translation = settingPosition + glm::vec2{0.0f, 6.0f};
    m_settingOverlay->m_Transform.scale = settingScale;
    m_settingOverlay->SetVisible(true);

    AddElements(m_bird);
    AddElements(m_playbutton);
    AddElements(m_exitbutton);
    AddElements(m_settingbutton);
    AddElements(m_settingOverlay);
}

void IntroScene::Update()
{
    if (m_settingOverlayIsAnimating && m_settingOverlay)
    {
        float &rotation = m_settingOverlay->m_Transform.rotation;
        const float delta = 0.2f;
        float remaining = m_settingOverlayTargetRotation - rotation;
        if (std::fabs(remaining) <= delta)
        {
            rotation = m_settingOverlayTargetRotation;
            m_settingOverlayIsAnimating = false;
        }
        else
        {
            rotation += (remaining > 0 ? delta : -delta);
        }
    }

    // Hover scale for settings button overlay (030) when hovering base (068)
    auto mousePos = Util::Input::GetCursorPosition();
    if (m_settingbutton && m_settingOverlay && m_settingbutton->IsHovering(mousePos)) {
        m_settingOverlay->m_Transform.scale = {0.9f, 0.9f};
    } else if (m_settingOverlay) {
        m_settingOverlay->m_Transform.scale = {0.8f, 0.8f};
    }

    m_movingBg->Update();
    Scene::Update();
}
