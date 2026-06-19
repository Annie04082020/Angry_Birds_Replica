#include "ui/GameHud.hpp"

#include "Resource.hpp"
#include "Util/Color.hpp"
#include "Util/Text.hpp"
#include "config.hpp"

#include <sstream>

namespace
{
    constexpr const char *kUIFont = RESOURCE_DIR "/font/angrybirds-regular.ttf";
    constexpr float kButtonScale = 0.85f;
    constexpr float kLeftPadding = 50.0f;
    constexpr float kTopPadding = 50.0f;
    constexpr float kButtonSpacing = 92.0f;
    constexpr float kScoreOffsetX = -50.0f;
    constexpr float kScoreLabelOffsetY = 1.0f;
    constexpr float kScoreValueOffsetY = -52.0f;
    constexpr int kScoreLabelSize = 57;
    constexpr int kScoreValueSize = 57;
    constexpr float kHighScoreOffsetX = -50.0f;
    constexpr float kHighScoreLabelOffsetY = -116.0f;
    constexpr float kHighScoreValueOffsetY = -164.0f;
    constexpr int kHighScoreLabelSize = 37;
    constexpr int kHighScoreValueSize = 57;
    const Util::Color kTextFillColor = Util::Color::FromRGB(245, 245, 245);
    const Util::Color kTextOutlineColor = Util::Color::FromRGB(24, 24, 24);
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

    void PositionOutlinedTextObjects(const glm::vec2 &position,
                                     std::array<std::shared_ptr<Util::GameObject>, 4> &outlineObjects,
                                     const std::shared_ptr<Util::GameObject> &frontObject,
                                     const float zoom)
    {
        for (size_t i = 0; i < outlineObjects.size(); ++i)
        {
            if (outlineObjects[i])
            {
                outlineObjects[i]->m_Transform.translation = position + kOutlineOffsets[i] / zoom;
                outlineObjects[i]->m_Transform.scale = {1.0f / zoom, 1.0f / zoom};
            }
        }

        if (frontObject)
        {
            frontObject->m_Transform.translation = position;
            frontObject->m_Transform.scale = {1.0f / zoom, 1.0f / zoom};
        }
    }
}

void GameHud::Build(const AddElementFunction &addElement, const float hudScale)
{
    if (m_HasBuilt)
    {
        return;
    }

    CreateOutlinedTextObjects("SCORE", static_cast<int>(kScoreLabelSize * hudScale), {0.0f, 0.0f}, 95.0f,
                              kTextFillColor, kTextOutlineColor, m_ScoreLabelOutline, m_ScoreLabel);
    for (const auto &outline : m_ScoreLabelOutline)
    {
        addElement(outline);
    }
    addElement(m_ScoreLabel);

    CreateOutlinedTextObjects("0", static_cast<int>(kScoreValueSize * hudScale), {0.0f, 0.0f}, 95.0f,
                              kTextFillColor, kTextOutlineColor, m_ScoreValueOutline, m_ScoreValue,
                              &m_ScoreValueDrawable, &m_ScoreValueOutlineDrawables);
    for (const auto &outline : m_ScoreValueOutline)
    {
        addElement(outline);
    }
    addElement(m_ScoreValue);

    CreateOutlinedTextObjects("HIGHSCORE", static_cast<int>(kHighScoreLabelSize * hudScale), {0.0f, 0.0f}, 95.0f,
                              kTextFillColor, kTextOutlineColor, m_HighScoreLabelOutline, m_HighScoreLabel);
    for (const auto &outline : m_HighScoreLabelOutline)
    {
        addElement(outline);
    }
    addElement(m_HighScoreLabel);

    CreateOutlinedTextObjects("0", static_cast<int>(kHighScoreValueSize * hudScale), {0.0f, 0.0f}, 95.0f,
                              kTextFillColor, kTextOutlineColor, m_HighScoreValueOutline, m_HighScoreValue,
                              &m_HighScoreValueDrawable, &m_HighScoreValueOutlineDrawables);
    for (const auto &outline : m_HighScoreValueOutline)
    {
        addElement(outline);
    }
    addElement(m_HighScoreValue);

    m_PauseButton = std::make_shared<Button>(Resource::Game_Button_093);
    m_PauseButton->SetZIndex(95.0f);
    m_PauseButton->SetScale({kButtonScale * hudScale, kButtonScale * hudScale});
    m_PauseButton->SetVisible(true);
    m_PauseButton->SetSFX(Resource::SETTING_SFX);
    m_PauseButton->SetOnClickFunction([this]() {
        if (m_OnTogglePause)
        {
            m_OnTogglePause();
        }
    });
    addElement(m_PauseButton);

    m_RestartButton = std::make_shared<Button>(Resource::Game_Button_031);
    m_RestartButton->SetZIndex(95.0f);
    m_RestartButton->SetScale({kButtonScale * hudScale, kButtonScale * hudScale});
    m_RestartButton->SetVisible(true);
    m_RestartButton->SetOnClickFunction([this]() {
        if (m_OnRestart)
        {
            m_OnRestart();
        }
    });
    addElement(m_RestartButton);

    m_HasBuilt = true;
}

void GameHud::UpdateLayout(const glm::vec2 &cameraPos,
                           const glm::vec2 &viewportSize,
                           const float hudScale,
                           const float zoom)
{
    if (!m_HasBuilt)
    {
        return;
    }

    const glm::vec2 topLeftAnchor = cameraPos +
                                    glm::vec2{
                                        -viewportSize.x * 0.5f / zoom + (kLeftPadding * hudScale) / zoom,
                                        viewportSize.y * 0.5f / zoom - (kTopPadding * hudScale) / zoom};

    if (m_PauseButton)
    {
        m_PauseButton->SetPosition(topLeftAnchor);
        m_PauseButton->SetScale({kButtonScale * hudScale / zoom, kButtonScale * hudScale / zoom});
    }

    if (m_RestartButton)
    {
        m_RestartButton->SetPosition(topLeftAnchor + glm::vec2{(kButtonSpacing * hudScale) / zoom, 0.0f});
        m_RestartButton->SetScale({kButtonScale * hudScale / zoom, kButtonScale * hudScale / zoom});
    }

    const glm::vec2 topRightAnchor = cameraPos +
                                     glm::vec2{
                                         viewportSize.x * 0.5f / zoom - (kLeftPadding * hudScale) / zoom,
                                         viewportSize.y * 0.5f / zoom - (kTopPadding * hudScale) / zoom};

    const glm::vec2 scoreAnchor = topRightAnchor + glm::vec2{(kScoreOffsetX * hudScale) / zoom, 0.0f};
    PositionOutlinedTextObjects(scoreAnchor + glm::vec2{0.0f, (kScoreLabelOffsetY * hudScale) / zoom},
                                m_ScoreLabelOutline, m_ScoreLabel, zoom);
    PositionOutlinedTextObjects(scoreAnchor + glm::vec2{0.0f, (kScoreValueOffsetY * hudScale) / zoom},
                                m_ScoreValueOutline, m_ScoreValue, zoom);

    const glm::vec2 highScoreAnchor = topRightAnchor + glm::vec2{(kHighScoreOffsetX * hudScale) / zoom, 0.0f};
    PositionOutlinedTextObjects(highScoreAnchor + glm::vec2{0.0f, (kHighScoreLabelOffsetY * hudScale) / zoom},
                                m_HighScoreLabelOutline, m_HighScoreLabel, zoom);
    PositionOutlinedTextObjects(highScoreAnchor + glm::vec2{0.0f, (kHighScoreValueOffsetY * hudScale) / zoom},
                                m_HighScoreValueOutline, m_HighScoreValue, zoom);
}

void GameHud::SetScore(const int score)
{
    const std::string scoreText = FormatScore(score);
    if (m_ScoreValueDrawable)
    {
        m_ScoreValueDrawable->SetText(scoreText);
    }
    for (const auto &drawable : m_ScoreValueOutlineDrawables)
    {
        if (drawable)
        {
            drawable->SetText(scoreText);
        }
    }
}

void GameHud::SetHighScore(const int highScore)
{
    const std::string highScoreText = FormatScore(highScore);
    if (m_HighScoreValueDrawable)
    {
        m_HighScoreValueDrawable->SetText(highScoreText);
    }
    for (const auto &drawable : m_HighScoreValueOutlineDrawables)
    {
        if (drawable)
        {
            drawable->SetText(highScoreText);
        }
    }
}

void GameHud::SetControlsVisible(const bool visible)
{
    if (m_PauseButton)
    {
        m_PauseButton->SetVisible(visible);
    }
    if (m_RestartButton)
    {
        m_RestartButton->SetVisible(visible);
    }
}
