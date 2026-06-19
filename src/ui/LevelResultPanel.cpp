#include "ui/LevelResultPanel.hpp"

#include "Resource.hpp"
#include "Util/Color.hpp"
#include "Util/DebugBox.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"
#include "config.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>

namespace
{
    constexpr const char *kUIFont = RESOURCE_DIR "/font/angrybirds-regular.ttf";
    constexpr int kClearTitleSize = 80;
    constexpr int kClearScoreValueSize = 64;
    constexpr int kClearHighScoreValueSize = 34;
    constexpr float kPanelWidth = 540.0f;
    constexpr float kPanelHeightRatio = 0.98f;
    constexpr float kClearStarScale = 0.7f;
    constexpr float kClearEmptyStarOpacity = 0.1f;
    constexpr float kClearBestStarScale = 0.1f;
    constexpr float kClearStarSpacing = 145.0f;
    constexpr float kClearBestStarSpacing = 24.0f;
    constexpr float kClearTitleOffsetY = 205.0f;
    constexpr float kClearStarsOffsetY = 72.0f;
    constexpr float kClearScoreOffsetY = -55.0f;
    constexpr float kClearHighScoreOffsetY = -130.0f;
    constexpr float kClearBestStarsOffsetX = 92.0f;
    constexpr float kClearButtonScale = 0.9f;
    constexpr float kClearButtonSpacing = 185.0f;
    constexpr float kClearButtonBaseOffsetY = -315.0f;
    constexpr float kClearStarPopDelay = 0.34f;
    constexpr float kClearStarPopDuration = 0.5f;
    constexpr float kClearStarPopYOffset = 30.0f;
    constexpr int kFailedTitleSize = 82;
    constexpr float kFailedTitleOffsetY = 200.0f;
    constexpr float kFailedPigScale = 2.6f;
    constexpr float kFailedPigOffsetY = 0.0f;
    constexpr float kFailedButtonScale = 0.9f;
    constexpr float kFailedButtonSpacing = 185.0f;
    constexpr float kFailedButtonBaseOffsetY = -315.0f;
    const Util::Color kTextFillColor = Util::Color::FromRGB(245, 245, 245);
    const Util::Color kTextOutlineColor = Util::Color::FromRGB(24, 24, 24);
    const Util::Color kScoreFillColor = Util::Color::FromRGB(255, 216, 82);
    const Util::Color kScoreOutlineColor = Util::Color::FromRGB(184, 102, 21);
    constexpr std::array<glm::vec2, 4> kOutlineOffsets = {
        glm::vec2{-2.5f, 0.0f},
        glm::vec2{2.5f, 0.0f},
        glm::vec2{0.0f, -2.5f},
        glm::vec2{0.0f, 2.5f}};

    std::string FormatScore(const int score)
    {
        std::ostringstream stream;
        stream << score;
        return stream.str();
    }

    float ComputeStarPopScale(const float elapsedTime)
    {
        if (elapsedTime <= 0.0f)
        {
            return 0.0f;
        }

        const float progress = std::clamp(elapsedTime / kClearStarPopDuration, 0.0f, 1.0f);
        const float overshoot = 1.0f + std::sin(progress * 3.1415926f) * 0.42f;
        return overshoot * progress;
    }

    std::shared_ptr<Util::GameObject> CreateTextObject(const std::string &text,
                                                       const int fontSize,
                                                       const glm::vec2 &position,
                                                       const float zIndex,
                                                       const Util::Color &color,
                                                       std::shared_ptr<Util::Text> *outDrawable = nullptr)
    {
        auto drawable = std::make_shared<Util::Text>(kUIFont, fontSize, text, color);
        if (outDrawable)
        {
            *outDrawable = drawable;
        }

        auto object = std::make_shared<Util::GameObject>(drawable, zIndex);
        object->m_Transform.translation = position;
        return object;
    }

    void CreateOutlinedTextObjects(const std::string &text,
                                   const int fontSize,
                                   const glm::vec2 &position,
                                   const float zIndex,
                                   const Util::Color &fillColor,
                                   const Util::Color &outlineColor,
                                   std::array<std::shared_ptr<Util::GameObject>, 4> &outlineObjects,
                                   std::shared_ptr<Util::GameObject> &frontObject,
                                   std::shared_ptr<Util::Text> *frontDrawable = nullptr,
                                   std::array<std::shared_ptr<Util::Text>, 4> *outlineDrawables = nullptr)
    {
        for (size_t i = 0; i < outlineObjects.size(); ++i)
        {
            std::shared_ptr<Util::Text> drawable = nullptr;
            outlineObjects[i] = CreateTextObject(text, fontSize, position + kOutlineOffsets[i], zIndex - 0.1f, outlineColor, &drawable);
            if (outlineDrawables)
            {
                (*outlineDrawables)[i] = drawable;
            }
        }

        frontObject = CreateTextObject(text, fontSize, position, zIndex, fillColor, frontDrawable);
    }

    void UpdateOutlineDrawables(std::shared_ptr<Util::Text> *frontDrawable,
                                std::array<std::shared_ptr<Util::Text>, 4> *outlineDrawables,
                                const std::string &text)
    {
        if (frontDrawable && *frontDrawable && outlineDrawables)
        {
            (*frontDrawable)->SetText(text);
            for (const auto &drawable : *outlineDrawables)
            {
                if (drawable)
                {
                    drawable->SetText(text);
                }
            }
        }
    }
}

void LevelResultPanel::Build(const AddElementFunction &addElement, const float hudScale)
{
    if (m_HasBuilt)
    {
        return;
    }

    m_ClearBackdrop = std::make_shared<Util::GameObject>(
        std::make_shared<Util::DebugBox>(glm::vec4{0.0f, 0.0f, 0.0f, 0.75f}, 1.0f), 95.0f);
    m_ClearBackdrop->SetVisible(false);
    addElement(m_ClearBackdrop);

    m_ClearTitle = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Text>(kUIFont, static_cast<int>(kClearTitleSize * hudScale), "LEVEL CLEARED!", kTextFillColor), 98.0f);
    m_ClearTitle->SetVisible(false);
    addElement(m_ClearTitle);

    for (int i = 0; i < 3; ++i)
    {
        auto emptyStarDrawable = std::make_shared<Util::Image>(Resource::Star_Empty);
        emptyStarDrawable->SetOpacity(kClearEmptyStarOpacity);
        m_ClearStars[i] = std::make_shared<Util::GameObject>(emptyStarDrawable, 97.0f);
        m_ClearStars[i]->m_Transform.scale = {kClearStarScale * hudScale, kClearStarScale * hudScale};
        m_ClearStars[i]->SetVisible(false);
        addElement(m_ClearStars[i]);

        m_ClearEarnedStars[i] = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(Resource::Star_Filled), 97.2f);
        m_ClearEarnedStars[i]->m_Transform.scale = {kClearStarScale * hudScale, kClearStarScale * hudScale};
        m_ClearEarnedStars[i]->SetVisible(false);
        addElement(m_ClearEarnedStars[i]);
    }

    CreateOutlinedTextObjects(FormatScore(0), static_cast<int>(kClearScoreValueSize * hudScale), {0.0f, 0.0f}, 97.0f,
                              kScoreFillColor, kScoreOutlineColor, m_ClearScoreOutline, m_ClearScore,
                              &m_ClearScoreDrawable, &m_ClearScoreOutlineDrawables);
    for (const auto &outline : m_ClearScoreOutline)
    {
        outline->SetVisible(false);
        addElement(outline);
    }
    m_ClearScore->SetVisible(false);
    addElement(m_ClearScore);

    CreateOutlinedTextObjects("BEST " + FormatScore(0), static_cast<int>(kClearHighScoreValueSize * hudScale), {0.0f, 0.0f}, 97.0f,
                              kTextFillColor, kTextOutlineColor, m_ClearHighScoreOutline, m_ClearHighScore,
                              &m_ClearHighScoreDrawable, &m_ClearHighScoreOutlineDrawables);
    for (const auto &outline : m_ClearHighScoreOutline)
    {
        outline->SetVisible(false);
        addElement(outline);
    }
    m_ClearHighScore->SetVisible(false);
    addElement(m_ClearHighScore);

    for (int i = 0; i < 3; ++i)
    {
        m_ClearBestStars[i] = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(Resource::Star_Empty), 97.5f);
        m_ClearBestStars[i]->SetVisible(false);
        addElement(m_ClearBestStars[i]);
    }

    m_ClearMenuButton = std::make_shared<Button>(Resource::Game_Menu_Item_073);
    m_ClearMenuButton->SetZIndex(98.0f);
    m_ClearMenuButton->SetScale({kClearButtonScale * hudScale, kClearButtonScale * hudScale});
    m_ClearMenuButton->SetVisible(false);
    m_ClearMenuButton->SetSFX(Resource::SETTING_SFX);
    m_ClearMenuButton->SetOnClickFunction([this]() {
        if (m_OnOpenLevelSelect)
        {
            m_OnOpenLevelSelect();
        }
    });
    addElement(m_ClearMenuButton);

    m_ClearRestartButton = std::make_shared<Button>(Resource::Game_Menu_Item_082);
    m_ClearRestartButton->SetZIndex(98.0f);
    m_ClearRestartButton->SetScale({kClearButtonScale * hudScale, kClearButtonScale * hudScale});
    m_ClearRestartButton->SetVisible(false);
    m_ClearRestartButton->SetSFX(Resource::SETTING_SFX);
    m_ClearRestartButton->SetOnClickFunction([this]() {
        if (m_OnRestart)
        {
            m_OnRestart();
        }
    });
    addElement(m_ClearRestartButton);

    m_ClearNextButton = std::make_shared<Button>(Resource::Game_Menu_Item_078);
    m_ClearNextButton->SetZIndex(98.0f);
    m_ClearNextButton->SetScale({kClearButtonScale * hudScale, kClearButtonScale * hudScale});
    m_ClearNextButton->SetVisible(false);
    m_ClearNextButton->SetSFX(Resource::SETTING_SFX);
    m_ClearNextButton->SetOnClickFunction([this]() {
        if (m_OnNextLevel)
        {
            m_OnNextLevel();
        }
    });
    addElement(m_ClearNextButton);

    m_FailedBackdrop = std::make_shared<Util::GameObject>(
        std::make_shared<Util::DebugBox>(glm::vec4{0.0f, 0.0f, 0.0f, 0.75f}, 1.0f), 95.0f);
    m_FailedBackdrop->SetVisible(false);
    addElement(m_FailedBackdrop);

    m_FailedTitle = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Text>(kUIFont, static_cast<int>(kFailedTitleSize * hudScale), "LEVEL FAILED!", kTextFillColor), 98.0f);
    m_FailedTitle->SetVisible(false);
    addElement(m_FailedTitle);

    m_FailedPig = std::make_shared<Util::GameObject>(std::make_shared<Util::Image>(Resource::PIG_SMALL), 98.0f);
    m_FailedPig->m_Transform.scale = {kFailedPigScale * hudScale, kFailedPigScale * hudScale};
    m_FailedPig->SetVisible(false);
    addElement(m_FailedPig);

    m_FailedMenuButton = std::make_shared<Button>(Resource::Game_Menu_Item_073);
    m_FailedMenuButton->SetZIndex(98.0f);
    m_FailedMenuButton->SetScale({kFailedButtonScale * hudScale, kFailedButtonScale * hudScale});
    m_FailedMenuButton->SetVisible(false);
    m_FailedMenuButton->SetSFX(Resource::SETTING_SFX);
    m_FailedMenuButton->SetOnClickFunction([this]() {
        if (m_OnOpenLevelSelect)
        {
            m_OnOpenLevelSelect();
        }
    });
    addElement(m_FailedMenuButton);

    m_FailedRestartButton = std::make_shared<Button>(Resource::Game_Menu_Item_082);
    m_FailedRestartButton->SetZIndex(98.0f);
    m_FailedRestartButton->SetScale({kFailedButtonScale * hudScale, kFailedButtonScale * hudScale});
    m_FailedRestartButton->SetVisible(false);
    m_FailedRestartButton->SetSFX(Resource::SETTING_SFX);
    m_FailedRestartButton->SetOnClickFunction([this]() {
        if (m_OnRestart)
        {
            m_OnRestart();
        }
    });
    addElement(m_FailedRestartButton);

    m_FailedNextButton = std::make_shared<Button>(Resource::Game_Menu_Item_078);
    m_FailedNextButton->SetZIndex(98.0f);
    m_FailedNextButton->SetScale({kFailedButtonScale * hudScale, kFailedButtonScale * hudScale});
    m_FailedNextButton->SetVisible(false);
    m_FailedNextButton->SetSFX(Resource::SETTING_SFX);
    m_FailedNextButton->SetOnClickFunction([this]() {
        if (m_OnNextLevel)
        {
            m_OnNextLevel();
        }
    });
    addElement(m_FailedNextButton);

    m_HasBuilt = true;
}

void LevelResultPanel::Reset()
{
    const auto hide = [](const std::shared_ptr<Util::GameObject> &object) {
        if (object)
        {
            object->SetVisible(false);
        }
    };

    hide(m_ClearBackdrop);
    hide(m_ClearTitle);
    for (const auto &star : m_ClearStars) { hide(star); }
    for (const auto &star : m_ClearEarnedStars) { hide(star); }
    for (const auto &outline : m_ClearScoreOutline) { hide(outline); }
    hide(m_ClearScore);
    for (const auto &outline : m_ClearHighScoreOutline) { hide(outline); }
    hide(m_ClearHighScore);
    for (const auto &star : m_ClearBestStars) { hide(star); }
    hide(m_ClearMenuButton);
    hide(m_ClearRestartButton);
    hide(m_ClearNextButton);
    hide(m_FailedBackdrop);
    hide(m_FailedTitle);
    hide(m_FailedPig);
    hide(m_FailedMenuButton);
    hide(m_FailedRestartButton);
    hide(m_FailedNextButton);
}

void LevelResultPanel::ShowClear(const glm::vec2 &cameraPos,
                                 const glm::vec2 &viewportSize,
                                 const float hudScale,
                                 const float zoom,
                                 const int score,
                                 const int stars,
                                 const int highScore,
                                 const int highScoreStars,
                                 const float animationTime)
{
    if (m_ClearBackdrop)
    {
        m_ClearBackdrop->SetVisible(true);
        m_ClearBackdrop->m_Transform.translation = cameraPos;
        m_ClearBackdrop->m_Transform.scale = {(kPanelWidth * hudScale) / zoom, viewportSize.y * kPanelHeightRatio / zoom};
    }
    if (m_ClearTitle)
    {
        m_ClearTitle->SetVisible(true);
        m_ClearTitle->m_Transform.translation = cameraPos + glm::vec2{0.0f, (kClearTitleOffsetY * hudScale) / zoom};
    }

    const float starStartX = -((kClearStarSpacing * hudScale) / zoom);
    const float baseStarScale = (kClearStarScale * hudScale) / zoom;
    for (int i = 0; i < 3; ++i)
    {
        if (!m_ClearStars[i])
        {
            continue;
        }

        const glm::vec2 baseStarPosition = cameraPos + glm::vec2{
            starStartX + static_cast<float>(i) * ((kClearStarSpacing * hudScale) / zoom),
            (kClearStarsOffsetY * hudScale) / zoom};
        m_ClearStars[i]->SetVisible(true);
        m_ClearStars[i]->m_Transform.scale = {baseStarScale, baseStarScale};
        m_ClearStars[i]->m_Transform.translation = baseStarPosition;

        if (!m_ClearEarnedStars[i])
        {
            continue;
        }

        if (i < stars)
        {
            const float starElapsedTime = animationTime - static_cast<float>(i) * kClearStarPopDelay;
            const float popScale = ComputeStarPopScale(starElapsedTime);
            if (popScale > 0.0f)
            {
                const float normalizedProgress = std::clamp(starElapsedTime / kClearStarPopDuration, 0.0f, 1.0f);
                const float yOffset = (1.0f - normalizedProgress) * ((kClearStarPopYOffset * hudScale) / zoom);
                m_ClearEarnedStars[i]->SetVisible(true);
                m_ClearEarnedStars[i]->m_Transform.scale = {baseStarScale * popScale, baseStarScale * popScale};
                m_ClearEarnedStars[i]->m_Transform.translation = baseStarPosition + glm::vec2{0.0f, yOffset};
            }
            else
            {
                m_ClearEarnedStars[i]->SetVisible(false);
                m_ClearEarnedStars[i]->m_Transform.scale = {0.0f, 0.0f};
                m_ClearEarnedStars[i]->m_Transform.translation = baseStarPosition;
            }
        }
        else
        {
            m_ClearEarnedStars[i]->SetVisible(false);
            m_ClearEarnedStars[i]->m_Transform.scale = {0.0f, 0.0f};
            m_ClearEarnedStars[i]->m_Transform.translation = baseStarPosition;
        }
    }

    if (m_ClearScore && m_ClearScoreDrawable)
    {
        m_ClearScore->SetVisible(true);
        const std::string scoreText = FormatScore(score);
        UpdateOutlineDrawables(&m_ClearScoreDrawable, &m_ClearScoreOutlineDrawables, scoreText);
        for (const auto &outline : m_ClearScoreOutline)
        {
            outline->SetVisible(true);
        }
        m_ClearScore->m_Transform.translation = cameraPos + glm::vec2{0.0f, (kClearScoreOffsetY * hudScale) / zoom};
        for (size_t i = 0; i < m_ClearScoreOutline.size(); ++i)
        {
            if (m_ClearScoreOutline[i])
            {
                m_ClearScoreOutline[i]->m_Transform.translation = m_ClearScore->m_Transform.translation + (kOutlineOffsets[i] * hudScale) / zoom;
            }
        }
    }

    if (m_ClearHighScore && m_ClearHighScoreDrawable)
    {
        m_ClearHighScore->SetVisible(true);
        const std::string highScoreText = "BEST " + FormatScore(highScore);
        UpdateOutlineDrawables(&m_ClearHighScoreDrawable, &m_ClearHighScoreOutlineDrawables, highScoreText);
        for (const auto &outline : m_ClearHighScoreOutline)
        {
            outline->SetVisible(true);
        }
        m_ClearHighScore->m_Transform.translation = cameraPos + glm::vec2{0.0f, (kClearHighScoreOffsetY * hudScale) / zoom};
        for (size_t i = 0; i < m_ClearHighScoreOutline.size(); ++i)
        {
            if (m_ClearHighScoreOutline[i])
            {
                m_ClearHighScoreOutline[i]->m_Transform.translation = m_ClearHighScore->m_Transform.translation + (kOutlineOffsets[i] * hudScale) / zoom;
            }
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        if (!m_ClearBestStars[i])
        {
            continue;
        }
        m_ClearBestStars[i]->SetVisible(true);
        m_ClearBestStars[i]->m_Transform.scale = {(kClearBestStarScale * hudScale) / zoom, (kClearBestStarScale * hudScale) / zoom};
        m_ClearBestStars[i]->m_Transform.translation = cameraPos + glm::vec2{
            (kClearBestStarsOffsetX * hudScale) / zoom + static_cast<float>(i) * ((kClearBestStarSpacing * hudScale) / zoom),
            (kClearHighScoreOffsetY * hudScale) / zoom};
        m_ClearBestStars[i]->SetDrawable(std::make_shared<Util::Image>(i < highScoreStars ? Resource::Star_Filled : Resource::Star_Empty));
    }

    if (m_ClearMenuButton)
    {
        m_ClearMenuButton->SetVisible(true);
        m_ClearMenuButton->SetPosition(cameraPos + glm::vec2{-(kClearButtonSpacing * hudScale) / zoom, (kClearButtonBaseOffsetY * hudScale) / zoom});
        m_ClearMenuButton->SetScale({kClearButtonScale * hudScale / zoom, kClearButtonScale * hudScale / zoom});
    }
    if (m_ClearRestartButton)
    {
        m_ClearRestartButton->SetVisible(true);
        m_ClearRestartButton->SetPosition(cameraPos + glm::vec2{0.0f, (kClearButtonBaseOffsetY * hudScale) / zoom});
        m_ClearRestartButton->SetScale({kClearButtonScale * hudScale / zoom, kClearButtonScale * hudScale / zoom});
    }
    if (m_ClearNextButton)
    {
        m_ClearNextButton->SetVisible(true);
        m_ClearNextButton->SetPosition(cameraPos + glm::vec2{(kClearButtonSpacing * hudScale) / zoom, (kClearButtonBaseOffsetY * hudScale) / zoom});
        m_ClearNextButton->SetScale({kClearButtonScale * hudScale / zoom, kClearButtonScale * hudScale / zoom});
    }
}

void LevelResultPanel::ShowFailed(const glm::vec2 &cameraPos,
                                  const glm::vec2 &viewportSize,
                                  const float hudScale,
                                  const float zoom)
{
    if (m_FailedBackdrop)
    {
        m_FailedBackdrop->SetVisible(true);
        m_FailedBackdrop->m_Transform.translation = cameraPos;
        m_FailedBackdrop->m_Transform.scale = {(kPanelWidth * hudScale) / zoom, viewportSize.y * kPanelHeightRatio / zoom};
    }
    if (m_FailedTitle)
    {
        m_FailedTitle->SetVisible(true);
        m_FailedTitle->m_Transform.translation = cameraPos + glm::vec2{0.0f, (kFailedTitleOffsetY * hudScale) / zoom};
    }
    if (m_FailedPig)
    {
        m_FailedPig->SetVisible(true);
        m_FailedPig->m_Transform.translation = cameraPos + glm::vec2{0.0f, (kFailedPigOffsetY * hudScale) / zoom};
        m_FailedPig->m_Transform.scale = {(kFailedPigScale * hudScale) / zoom, (kFailedPigScale * hudScale) / zoom};
    }
    if (m_FailedMenuButton)
    {
        m_FailedMenuButton->SetVisible(true);
        m_FailedMenuButton->SetPosition(cameraPos + glm::vec2{-(kFailedButtonSpacing * hudScale) / zoom, (kFailedButtonBaseOffsetY * hudScale) / zoom});
        m_FailedMenuButton->SetScale({kFailedButtonScale * hudScale / zoom, kFailedButtonScale * hudScale / zoom});
    }
    if (m_FailedRestartButton)
    {
        m_FailedRestartButton->SetVisible(true);
        m_FailedRestartButton->SetPosition(cameraPos + glm::vec2{0.0f, (kFailedButtonBaseOffsetY * hudScale) / zoom});
        m_FailedRestartButton->SetScale({kFailedButtonScale * hudScale / zoom, kFailedButtonScale * hudScale / zoom});
    }
    if (m_FailedNextButton)
    {
        m_FailedNextButton->SetVisible(true);
        m_FailedNextButton->SetPosition(cameraPos + glm::vec2{(kFailedButtonSpacing * hudScale) / zoom, (kFailedButtonBaseOffsetY * hudScale) / zoom});
        m_FailedNextButton->SetScale({kFailedButtonScale * hudScale / zoom, kFailedButtonScale * hudScale / zoom});
    }
}
