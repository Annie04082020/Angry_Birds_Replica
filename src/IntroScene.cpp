#include "IntroScene.hpp"
#include "BGM.hpp"
#include "Resource.hpp"

std::shared_ptr<IntroScene> IntroScene::Create()
{
    auto bg = std::make_shared<DynamicBackground>(Resource::MOVING_BG_IMAGE);
    return std::shared_ptr<IntroScene>(new IntroScene(bg));
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
        m_bird->SetVisible(true); });

    m_exitbutton = std::make_shared<Button>(Resource::Exit_Button);
    m_exitbutton->SetZIndex(50);
    m_exitbutton->SetPosition({-1000, -550});
    m_exitbutton->SetVisible(true);
    m_exitbutton->SetOnClickFunction([this]()
                                     {
        m_exitbutton->SetVisible(false);
        m_bird->SetVisible(true); });

    m_settingbutton = std::make_shared<Button>(Resource::Setting_Button);
    m_settingbutton->SetZIndex(50);
    m_settingbutton->SetPosition({1000, -550});
    m_settingbutton->SetVisible(true);
    m_settingbutton->SetSFX(Resource::SETTING_SFX);
    m_settingbutton->SetOnClickFunction([this]()
                                        { m_bird->SetVisible(true); });

    AddElements(m_bird);
    AddElements(m_playbutton);
    AddElements(m_exitbutton);
    AddElements(m_settingbutton);
}

void IntroScene::Update()
{
    m_movingBg->Update();
    Scene::Update();
}
