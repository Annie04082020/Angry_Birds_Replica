#include "LevelSelectScene.hpp"

#include "BGM.hpp"
#include "Resource.hpp"
#include "Util/Color.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"

#include <string>

namespace
{
    constexpr const char *kUIFont = RESOURCE_DIR "/font/angrybirds-regular.ttf";
    constexpr int kTotalLevels = 10;
    constexpr float kLabelHoverScaleMultiplier = 1.125f;
    constexpr float kLevelSelectScale = 1.3f;
    constexpr float kLevelSelectButtonScale = 0.8f * kLevelSelectScale;
    constexpr float kLevelSelectBackButtonScale = 0.85f * kLevelSelectScale;
    constexpr int kLevelSelectTitleSize = static_cast<int>(40 * kLevelSelectScale);
    constexpr int kLevelSelectSubtitleSize = static_cast<int>(22 * kLevelSelectScale);
    constexpr int kLevelSelectNumberSize = static_cast<int>(34 * kLevelSelectScale);
    constexpr int kLevelSelectBackLabelSize = static_cast<int>(24 * kLevelSelectScale);

    std::shared_ptr<Util::GameObject> CreateTextObject(const std::string &text,
                                                       const int fontSize,
                                                       const glm::vec2 &position,
                                                       const float zIndex,
                                                       const Util::Color &color)
    {
        auto drawable = std::make_shared<Util::Text>(kUIFont, fontSize, text, color);
        auto object = std::make_shared<Util::GameObject>(drawable, zIndex);
        object->m_Transform.translation = position;
        return object;
    }
}

std::shared_ptr<LevelSelectScene> LevelSelectScene::Create()
{
    auto bg = std::make_shared<DynamicBackground>(Resource::MOVING_BG_IMAGE);
    struct Enabler : public LevelSelectScene
    {
        Enabler(std::shared_ptr<DynamicBackground> bg) : LevelSelectScene(bg) {}
    };
    return std::make_shared<Enabler>(bg);
}

LevelSelectScene::LevelSelectScene(std::shared_ptr<DynamicBackground> bg)
    : Scene(bg), m_movingBg(bg)
{
    SetVisible(true);
    SetZIndex(55);
    SetBGM(std::make_shared<BackgroundMusic>(Resource::TITLE_THEME));
    BuildLevelSelectUI();
    SetSceneVisible(false);
}

void LevelSelectScene::BuildLevelSelectUI()
{
    const float startX = -300.0f;
    const float startY = 110.0f;
    const float stepX = 115.0f * kLevelSelectScale;
    const float stepY = 120.0f * kLevelSelectScale;

    m_title = CreateTextObject(
        "SELECT LEVEL", kLevelSelectTitleSize, {10.0f, 230.0f}, 90, Util::Color::FromRGB(255, 255, 255));
    AddElements(m_title);

    m_backButton = std::make_shared<Button>(Resource::Additional_Button_Base);
    m_backButton->SetZIndex(88);
    m_backButton->SetPosition({0.0f, -200.0f});
    m_backButton->SetScale({kLevelSelectBackButtonScale, kLevelSelectBackButtonScale});
    m_backButton->SetSFX(Resource::SETTING_SFX);
    m_backButton->SetOnClickFunction([this]()
                                     {
        if (m_onBackClick)
        {
            m_onBackClick();
        } });
    AddElements(m_backButton);

    m_backLabel = CreateTextObject(
        "BACK", kLevelSelectBackLabelSize, {7.0f, -190.0f}, 89, Util::Color::FromRGB(255, 255, 255));
    m_backLabel->m_Transform.scale = m_backLabelBaseScale;
    AddElements(m_backLabel);

    for (int levelIndex = 0; levelIndex < kTotalLevels; ++levelIndex)
    {
        const int levelNumber = levelIndex + 1;
        const int column = levelIndex % 5;
        const int row = levelIndex / 5;
        const glm::vec2 buttonPosition = {
            startX + static_cast<float>(column) * stepX,
            startY - static_cast<float>(row) * stepY};

        auto levelButton = std::make_shared<Button>(Resource::Setting_Button_Base);
        levelButton->SetZIndex(88);
        levelButton->SetPosition(buttonPosition);
        levelButton->SetScale({kLevelSelectButtonScale, kLevelSelectButtonScale});
        levelButton->SetSFX(Resource::SETTING_SFX);

        levelButton->SetOnClickFunction([this, levelNumber]()
                                        {
            if (m_onLevelSelect)
            {
                m_onLevelSelect(levelNumber);
            } });
        AddElements(levelButton);
        m_levelButtons.push_back(levelButton);

        const std::string labelText = std::to_string(levelNumber);
        const int labelSize = kLevelSelectNumberSize;

        auto label = CreateTextObject(
            labelText, labelSize, buttonPosition + glm::vec2{7.0f, 10.0f}, 89, Util::Color::FromRGB(255, 255, 255));
        label->m_Transform.scale = {1.0f, 1.0f};
        AddElements(label);
        m_levelLabels.push_back(label);
        m_levelLabelBaseScales.push_back({1.0f, 1.0f});
    }
}

void LevelSelectScene::Update()
{
    const auto mousePos = Util::Input::GetCursorPosition();

    if (m_backButton && m_backLabel)
    {
        const bool isBackHovered = m_backButton->GetVisibility() && m_backButton->IsHovering(mousePos);
        m_backLabel->m_Transform.scale = isBackHovered
                                             ? m_backLabelBaseScale * kLabelHoverScaleMultiplier
                                             : m_backLabelBaseScale;
    }

    const size_t hoverCount = glm::min(m_levelButtons.size(), m_levelLabels.size());
    for (size_t i = 0; i < hoverCount; ++i)
    {
        const bool isHovered = m_levelButtons[i] &&
                               m_levelButtons[i]->GetVisibility() &&
                               m_levelButtons[i]->IsHovering(mousePos);
        m_levelLabels[i]->m_Transform.scale = isHovered
                                                  ? m_levelLabelBaseScales[i] * kLabelHoverScaleMultiplier
                                                  : m_levelLabelBaseScales[i];
    }

    if (m_movingBg)
    {
        m_movingBg->Update();
    }

    Scene::Update();
}

void LevelSelectScene::SetSceneVisible(const bool visible)
{
    if (m_title)
    {
        m_title->SetVisible(visible);
    }
    if (m_subtitle)
    {
        m_subtitle->SetVisible(visible);
    }
    if (m_backButton)
    {
        m_backButton->SetVisible(visible);
    }
    if (m_backLabel)
    {
        m_backLabel->SetVisible(visible);
    }

    for (const auto &button : m_levelButtons)
    {
        button->SetVisible(visible);
    }
    for (const auto &label : m_levelLabels)
    {
        label->SetVisible(visible);
    }
}
