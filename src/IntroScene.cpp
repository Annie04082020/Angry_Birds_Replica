#include "IntroScene.hpp"
#include "BGM.hpp"
#include "Resource.hpp"
#include "Util/Image.hpp"

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
    SetVisible(false);
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

    m_settingbutton = std::make_shared<Button>(Resource::Setting_Button_Base);
    m_settingbutton->SetZIndex(50);
    m_settingbutton->SetPosition({450, -300});
    m_settingbutton->SetVisible(true);
    m_settingbutton->SetSFX(Resource::SETTING_SFX);
    m_settingbutton->SetOnClickFunction([this]()
                                        { 
        m_bird->SetVisible(true);
        m_isRotating = true;
        m_targetRotation = m_overlayRotation + 180.0f;
    });

    m_settingOverlay = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Button_Overlay), 51);
    m_settingOverlay->m_Transform.translation = {450, -293};
    m_settingOverlay->SetVisible(true);

    AddElements(m_bird);
    AddElements(m_playbutton);
    AddElements(m_exitbutton);
    AddElements(m_settingbutton);
    AddElements(m_settingOverlay);
}

void IntroScene::Update()
{
    // Update rotation animation
    if (m_isRotating) {
        m_overlayRotation += m_rotationSpeed;
        if (m_overlayRotation >= m_targetRotation) {
            m_overlayRotation = m_targetRotation;
            m_isRotating = false;
        }
    }
    m_settingOverlay->m_Transform.rotation = glm::radians(m_overlayRotation);
    
    // Sync setting overlay scale with button
    m_settingOverlay->m_Transform.scale = m_settingbutton->m_Transform.scale;
    
    m_movingBg->Update();
    Scene::Update();
}
