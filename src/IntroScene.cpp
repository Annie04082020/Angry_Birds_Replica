#include "IntroScene.hpp"
#include "BGM.hpp"
#include "Resource.hpp"

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

    m_playbutton = std::make_shared<Button>(Resource::Play_Button);
    m_playbutton->SetZIndex(50);
    m_playbutton->SetVisible(true);
    m_playbutton->SetOnClickFunction([this]()
                                     {
        if (m_onPlayClick && m_onPlayClick()) {
            m_playbutton->SetVisible(false);
            if (m_exitbutton) m_exitbutton->SetVisible(false);
            if (m_settingbutton) m_settingbutton->SetVisible(false);
        } });

    m_exitbutton = std::make_shared<Button>(Resource::Exit_Button);
    m_exitbutton->SetZIndex(50);
    m_exitbutton->SetPosition({-1000, -550});
    m_exitbutton->SetVisible(true);
    m_exitbutton->SetOnClickFunction([this]() {});

    m_settingbutton = std::make_shared<Button>(Resource::Setting_Button);
    m_settingbutton->SetZIndex(50);
    m_settingbutton->SetPosition({1000, -550});
    m_settingbutton->SetVisible(true);
    m_settingbutton->SetSFX(Resource::SETTING_SFX);
    m_settingbutton->SetOnClickFunction([this]() {});

    AddElements(m_playbutton);
    AddElements(m_exitbutton);
    AddElements(m_settingbutton);
}

void IntroScene::Update()
{
    m_movingBg->Update();
    Scene::Update();
}
