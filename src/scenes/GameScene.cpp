#include "GameScene.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "config.hpp"

#include "Resource.hpp"
#include "Util/Color.hpp"
#include "Util/DebugBox.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Text.hpp"
#include "Util/Time.hpp"
#include "Util/TransformUtils.hpp"
#include <SDL_mixer.h>

namespace
{
    constexpr const char *kUIFont = RESOURCE_DIR "/font/angrybirds-regular.ttf";
    constexpr const char *kHighScoreFilePath = RESOURCE_DIR "/high_scores.json";
    constexpr float kGrassTopRatio = 404.0f / 563.0f;
    std::string GetHudLabel(const std::shared_ptr<Character> &object)
    {
        if (!object)
        {
            return "OBJECT";
        }

        std::string label = object->GetBaseImageId().empty() ? object->GetImagePath() : object->GetBaseImageId();
        const std::size_t slashPos = label.find_last_of("/\\");
        if (slashPos != std::string::npos)
        {
            label = label.substr(slashPos + 1);
        }

        const std::size_t dotPos = label.find_last_of('.');
        if (dotPos != std::string::npos)
        {
            label = label.substr(0, dotPos);
        }

        return label.empty() ? "OBJECT" : label;
    }

    std::string BuildDamageHudText(const std::vector<std::shared_ptr<Character>> &objects)
    {
        std::ostringstream out;
        out << "Damage test\n";

        std::unordered_map<std::string, int> occurrenceCount;
        for (const auto &object : objects)
        {
            if (!object)
            {
                continue;
            }

            const std::string baseId = GetHudLabel(object);
            const int currentIndex = ++occurrenceCount[baseId];
            const float maxHealth = object->GetMaxHealth();
            const float health = object->GetHealth();
            const float damagePercent = maxHealth > 0.0f ? std::max(0.0f, (maxHealth - health) / maxHealth * 100.0f) : 0.0f;

            out << currentIndex << ". " << baseId << "  dmg: " << static_cast<int>(std::round(damagePercent)) << "%\n";
        }

        return out.str();
    }

    constexpr float kGameHudButtonScale = 0.85f;
    constexpr float kGameHudLeftPadding = 50.0f;
    constexpr float kGameHudTopPadding = 50.0f;
    constexpr float kGameHudButtonSpacing = 92.0f;
    constexpr float kGameHudScoreOffsetX = -50.0f;
    constexpr float kGameHudScoreLabelOffsetY = 1.0f;
    constexpr float kGameHudScoreValueOffsetY = -52.0f;
    constexpr int kGameHudScoreLabelSize = 57;
    constexpr int kGameHudScoreValueSize = 57;
    constexpr float kGameHudHighScoreOffsetX = -50.0f;
    constexpr float kGameHudHighScoreLabelOffsetY = -116.0f;
    constexpr float kGameHudHighScoreValueOffsetY = -164.0f;
    constexpr int kGameHudHighScoreLabelSize = 37;
    constexpr int kGameHudHighScoreValueSize = 57;
    const Util::Color kGameHudTextFillColor = Util::Color::FromRGB(245, 245, 245);
    const Util::Color kGameHudTextOutlineColor = Util::Color::FromRGB(24, 24, 24);
    constexpr std::array<glm::vec2, 4> kGameHudOutlineOffsets = {
        glm::vec2{-2.5f, 0.0f},
        glm::vec2{2.5f, 0.0f},
        glm::vec2{0.0f, -2.5f},
        glm::vec2{0.0f, 2.5f}};
    constexpr float kGamePauseMenuBackdropWidth = 500.0f;
    constexpr float kGamePauseMenuBackdropHeightRatio = 0.99f;
    constexpr float kGamePauseMenuBackdropCenterOffsetX = 100.0f;
    constexpr float kGamePauseMenuBackdropCenterOffsetY = 10.0f;
    constexpr float kGamePauseMenu069OffsetX = 5.0f;
    constexpr float kGamePauseMenu069OffsetY = 5.0f;
    constexpr float kGamePauseMenu069Scale = 1.2f;
    constexpr float kGamePauseMenu082OffsetX = 125.0f;
    constexpr float kGamePauseMenu082OffsetY = 250.0f;
    constexpr float kGamePauseMenu082Scale = 1.25f;
    constexpr float kGamePauseMenu073OffsetX = 125.0f;
    constexpr float kGamePauseMenu073OffsetY = 400.0f;
    constexpr float kGamePauseMenu073Scale = 1.25f;
    constexpr float kGamePauseMenu005OffsetX = 90.0f;
    constexpr float kGamePauseMenu005OffsetY = 900.0f;
    constexpr float kGamePauseMenu005Scale = 1.2f;
    constexpr float kGamePauseMenu040OffsetX = 90.0f;
    constexpr float kGamePauseMenu040OffsetY = 895.0f;
    constexpr float kGamePauseMenu040Scale = 1.2f;
    constexpr float kGamePauseMenu063OffsetX = 180.0f;
    constexpr float kGamePauseMenu063OffsetY = 900.0f;
    constexpr float kGamePauseMenu063Scale = 1.2f;
    constexpr float kGamePauseMenuLevelTitleOffsetX = 280.0f;
    constexpr float kGamePauseMenuLevelTitleOffsetY = 450.0f;
    constexpr float kGamePauseMenuLevelTitleScale = 0.5f;

    // Level Clear Screen Constants
    constexpr int kLevelClearTitleSize = 80;
    constexpr int kLevelClearScoreValueSize = 64;
    constexpr int kLevelClearHighScoreValueSize = 34;
    constexpr float kLevelClearPanelWidth = 540.0f;
    constexpr float kLevelClearPanelHeightRatio = 0.98f;
    constexpr float kLevelClearStarScale = 0.7f;
    constexpr float kLevelClearEmptyStarOpacity = 0.1f;
    constexpr float kLevelClearBestStarScale = 0.1f;
    constexpr float kLevelClearStarSpacing = 145.0f;
    constexpr float kLevelClearBestStarSpacing = 24.0f;
    constexpr float kLevelClearTitleOffsetY = 205.0f;
    constexpr float kLevelClearStarsOffsetY = 72.0f;
    constexpr float kLevelClearScoreOffsetY = -55.0f;
    constexpr float kLevelClearHighScoreOffsetY = -130.0f;
    constexpr float kLevelClearBestStarsOffsetX = 92.0f;
    constexpr float kLevelClearButtonScale = 0.9f;
    constexpr float kLevelClearButtonSpacing = 185.0f;
    constexpr float kLevelClearButtonBaseOffsetY = -315.0f;
    constexpr float kLevelClearStarPopDelay = 0.34f;
    constexpr float kLevelClearStarPopDuration = 0.5f;
    constexpr float kLevelClearStarPopYOffset = 30.0f;
    
    constexpr int kLevelFailedTitleSize = 82;
    constexpr float kLevelFailedTitleOffsetY = 200.0f;
    constexpr float kLevelFailedPigScale = 2.6f;
    constexpr float kLevelFailedPigOffsetY = 0.0f;
    constexpr float kLevelFailedButtonScale = 0.9f;
    constexpr float kLevelFailedButtonSpacing = 185.0f;
    constexpr float kLevelFailedButtonBaseOffsetY = -315.0f;
    const Util::Color kLevelClearScoreFillColor = Util::Color::FromRGB(255, 216, 82);
    const Util::Color kLevelClearScoreOutlineColor = Util::Color::FromRGB(184, 102, 21);

    std::string FormatScore(const int score)
    {
        std::ostringstream stream;
        stream << score;
        return stream.str();
    }

    int ComputeStarCount(const int score)
    {
        if (score >= 45000)
        {
            return 3;
        }
        if (score >= 30000)
        {
            return 2;
        }
        if (score >= 15000)
        {
            return 1;
        }
        return 0;
    }

    float ComputeStarPopScale(const float elapsedTime)
    {
        if (elapsedTime <= 0.0f)
        {
            return 0.0f;
        }

        const float progress = std::clamp(elapsedTime / kLevelClearStarPopDuration, 0.0f, 1.0f);
        const float overshoot = 1.0f + std::sin(progress * 3.1415926f) * 0.42f;
        return overshoot * progress;
    }

    class FloatingTextObject : public Util::GameObject
    {
    public:
        FloatingTextObject(const std::shared_ptr<Util::Text> &drawable,
                           const glm::vec2 &position,
                           const glm::vec2 &velocity,
                           const Util::Color &baseColor,
                           const float zIndex,
                           const float lifeTime)
            : Util::GameObject(drawable, zIndex),
              m_DrawableText(drawable),
              m_Velocity(velocity),
              m_BaseColor(baseColor),
              m_LifeTime(lifeTime),
              m_RemainingLife(lifeTime)
        {
            m_Transform.translation = position;
        }

        void Update() override
        {
            if (!m_DrawableText || m_RemainingLife <= 0.0f)
            {
                return;
            }

            const float deltaSec = std::max(0.0f, Util::Time::GetDeltaTimeMs() / 1000.0f);
            m_RemainingLife = std::max(0.0f, m_RemainingLife - deltaSec);
            m_Transform.translation += m_Velocity * deltaSec;

            const float ratio = (m_LifeTime > 0.0f) ? (m_RemainingLife / m_LifeTime) : 0.0f;
            Util::Color fadedColor = m_BaseColor;
            fadedColor.a = std::max(0.0f, 255.0f * ratio);
            m_DrawableText->SetColor(fadedColor);
        }

    private:
        std::shared_ptr<Util::Text> m_DrawableText = nullptr;
        glm::vec2 m_Velocity{0.0f, 45.0f};
        Util::Color m_BaseColor = Util::Color::FromRGB(255, 255, 255);
        float m_LifeTime = 0.8f;
        float m_RemainingLife = 0.8f;
    };

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

    void CreateOutlinedTextObjects(
        const std::string &text,
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
            outlineObjects[i] = CreateTextObject(
                text,
                fontSize,
                position + kGameHudOutlineOffsets[i],
                zIndex - 0.1f,
                outlineColor,
                &drawable);

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
                outlineObjects[i]->m_Transform.translation = position + kGameHudOutlineOffsets[i] / zoom;
            }
        }

        if (frontObject)
        {
            frontObject->m_Transform.translation = position;
        }
    }

    void UpdateOutlineDrawables(std::shared_ptr<Util::Text> *frontDrawable,
                                std::array<std::shared_ptr<Util::Text>, 4> *outlineDrawables,
                                const std::string &text)
    {
        if (frontDrawable && *frontDrawable && outlineDrawables)
        {
            (*frontDrawable)->SetText(text);
            for (size_t i = 0; i < outlineDrawables->size(); ++i)
            {
                if ((*outlineDrawables)[i])
                {
                    (*outlineDrawables)[i]->SetText(text);
                }
            }
        }
    }

    float ComputeImpactSpeed(const std::shared_ptr<Character> &a,
                             const std::shared_ptr<Character> &b,
                             const glm::vec2 &contactNormal)
    {
        if (!(a && b))
        {
            return 0.0f;
        }

        const glm::vec2 relativeVelocity = b->GetVelocity() - a->GetVelocity();
        return std::max(0.0f, std::fabs(glm::dot(relativeVelocity, contactNormal)));
    }

    float GetDamageScale(const std::shared_ptr<Character> &target,
                         const std::shared_ptr<Character> &other)
    {
        if (!(target && other))
        {
            return 0.0f;
        }

        if (target->GetEntityKind() == Character::EntityKind::Pig)
        {
            return 0.22f;
        }

        float scale = 0.08f;
        switch (target->GetMaterialType())
        {
        case Character::MaterialType::Glass:
        case Character::MaterialType::Ice:
            scale = 0.12f;
            break;
        case Character::MaterialType::Wood:
            scale = 0.09f;
            break;
        case Character::MaterialType::Stone:
            scale = 0.05f;
            break;
        default:
            break;
        }

        if (other->GetEntityKind() == Character::EntityKind::Bird)
        {
            scale *= 1.2f;

            const std::string &baseId = other->GetBaseImageId();
            const std::string &path = other->GetImagePath();
            if ((baseId.find("BIRD_BLUE") != std::string::npos || path.find("blue") != std::string::npos) &&
                target->GetMaterialType() == Character::MaterialType::Stone)
            {
                scale *= 0.45f;
            }
            else if ((baseId.find("BIRD_YELLOW") != std::string::npos || path.find("yellow") != std::string::npos) &&
                     target->GetMaterialType() == Character::MaterialType::Wood)
            {
                scale *= 1.35f;
            }
        }
        else if (other->GetEntityKind() == Character::EntityKind::Environment)
        {
            scale *= 1.1f;
        }

        return scale;
    }

    bool IsDestructible(const std::shared_ptr<Character> &character)
    {
        if (!character || character->IsDestroyed() || !character->ParticipatesInPhysics())
        {
            return false;
        }

        if (character->GetEntityKind() == Character::EntityKind::Pig)
        {
            return true;
        }

        return character->GetEntityKind() == Character::EntityKind::Environment &&
               character->GetMaterialType() != Character::MaterialType::None &&
               character->GetMaxHealth() < 9000.0f;
    }
}

bool GameScene::LoadLevel(const std::string &levelPath)
{
    m_ZoomScrollAccumulator = 0.0f;
    m_DamageOutputTimer = 0.0f;
    m_ShowDamageHud = false;

    Util::SetCameraZoom(1.0f);
    Util::SetCameraPosition({0.0f, 0.0f});

    if (!m_LevelManager || !m_LevelManager->LoadLevel(levelPath))
    {
        return false;
    }

    const auto &objects = m_LevelManager->GetGameObjects();

    for (const auto &obj : objects)
    {
        AddElements(obj);
    }

    const glm::vec2 viewportSize = Util::GetViewportSize();
    const float floorCandidate = (0.5f - kGrassTopRatio) * viewportSize.y;
    this->SetWorldFloorY(floorCandidate);

    if (m_BirdLaunchController)
    {
        m_BirdLaunchController->SetWorldFloorY(floorCandidate);
        m_BirdLaunchController->LoadLevelObjects(objects);
    }

    const bool isDamageTestLevel = levelPath.find("_test") != std::string::npos ||
                                   m_LevelManager->GetLevelName().find("Test") != std::string::npos;
    if (isDamageTestLevel)
    {
        m_ShowDamageHud = true;
        // Output damage info to console instead of on-screen HUD
        std::cout << "\n=== Test Level Loaded: " << m_LevelManager->GetLevelName() << " ===\n";
        std::cout << BuildDamageHudText(objects) << std::endl;
    }

    m_ScoringSystem.LoadConfig(RESOURCE_DIR "/scoring_config.json");
    LoadLevelHighScore();

    ResetScoreState();
    BuildLevelHud();
    UpdateHudPositions();
    UpdateScoreHud();

    m_SceneInputController = std::make_shared<SceneInputController>(m_DynamicBackground, m_LevelManager);

    return true;
}

void GameScene::LoadLevelHighScore()
{
    if (!m_LevelManager)
    {
        m_ScoringSystem.SetHighScore(0);
        m_HudHighScore = 0;
        return;
    }

    m_ScoringSystem.LoadHighScoreFromFile(kHighScoreFilePath, m_LevelManager->GetLevel());
    m_HudHighScore = m_ScoringSystem.GetHighScore();
}

void GameScene::PersistLevelHighScore() const
{
    if (!m_LevelManager)
    {
        return;
    }

    m_ScoringSystem.SaveHighScoreToFile(kHighScoreFilePath, m_LevelManager->GetLevel());
}

void GameScene::Update()
{
    if (Util::Input::IsKeyUp(Util::Keycode::C))
    {
        m_LevelCleared = true;
        m_LevelFailed = false;
    }

    const bool isBirdHolding = m_BirdLaunchController && m_BirdLaunchController->IsHoldingBird();

    if (Util::Input::IsKeyPressed(Util::Keycode::F1))
    {
        SetDebugRenderEnabled(!IsDebugRenderEnabled());
        std::cout << "[Debug] Physics render "
                  << (IsDebugRenderEnabled() ? "enabled" : "disabled") << std::endl;
    }

    if (Util::Input::IsKeyPressed(Util::Keycode::P))
    {
        SetPhysicsPaused(!IsPhysicsPaused());
        std::cout << "[Debug] Physics "
                  << (IsPhysicsPaused() ? "paused" : "resumed") << std::endl;
    }

    if (IsPhysicsPaused() && Util::Input::IsKeyPressed(Util::Keycode::L))
    {
        RequestPhysicsSingleStep();
        std::cout << "[Debug] Physics single step dt=0.016" << std::endl;
    }

    // Output damage stats to console periodically during test level
    if (m_ShowDamageHud && m_LevelManager)
    {
        m_DamageOutputTimer += Util::Time::GetDeltaTimeMs() / 1000.0f;
        if (m_DamageOutputTimer >= 2.0f) // Output every 2 seconds
        {
            m_DamageOutputTimer = 0.0f;
            std::cout << "\n=== Damage Status ===\n"
                      << BuildDamageHudText(m_LevelManager->GetGameObjects()) << std::endl;
        }
    }

    if (m_BirdLaunchController && !IsPhysicsPaused())
    {
        m_BirdLaunchController->Update();
    }

    if (m_SceneInputController)
    {
        m_SceneInputController->Update(isBirdHolding);
    }

    if (m_PauseMenuInputBlockedUntilRelease &&
        !Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB))
    {
        m_PauseMenuInputBlockedUntilRelease = false;
        SetPauseMenuButtonsInputEnabled(true);
    }

    // Handle mouse wheel zoom with mouse position as pivot
    if (Util::Input::IfScroll())
    {
        const glm::vec2 mousePos = Util::Input::GetCursorPosition();
        const float oldZoom = Util::GetCameraZoom();
        const glm::vec2 oldCameraPos = Util::GetCameraPosition();
        const glm::vec2 scrollDist = Util::Input::GetScrollDistance();
        const float normalizedScroll = std::clamp(scrollDist.y, -1.0f, 1.0f);
        m_ZoomScrollAccumulator += normalizedScroll;

        const int zoomSteps = static_cast<int>(m_ZoomScrollAccumulator);
        if (zoomSteps != 0)
        {
            m_ZoomScrollAccumulator -= static_cast<float>(zoomSteps);

            // Calculate world position of mouse before zoom
            const glm::vec2 worldMousePos = mousePos / oldZoom + oldCameraPos;

            constexpr float zoomStep = 0.01f;
            const float stepScale = zoomSteps > 0 ? (1.0f + zoomStep)
                                                  : (1.0f - zoomStep);
            const float newZoom = oldZoom * std::pow(stepScale, std::abs(zoomSteps));
            Util::SetCameraZoom(newZoom);

            // Get actual zoom after clamp
            const float actualZoom = Util::GetCameraZoom();

            // Only adjust camera position if zoom actually changed (not clamped)
            if (actualZoom != oldZoom)
            {
                const glm::vec2 newCameraPos = worldMousePos - mousePos / actualZoom;
                glm::vec2 clampedCameraPos = newCameraPos;
                // Keep background bottom aligned with viewport bottom.
                clampedCameraPos.y = 0.0f;
                Util::SetCameraPosition(clampedCameraPos);
            }
        }
    }

    UpdateHudPositions();
    Scene::Update();
    RefreshRemainingPigCount();
    UpdateWinState();
    UpdateFailState();
}

void GameScene::BuildLevelHud()
{
    if (!m_ScoreLabel)
    {
        CreateOutlinedTextObjects("SCORE",
                                  kGameHudScoreLabelSize,
                                  {0.0f, 0.0f},
                                  95.0f,
                                  kGameHudTextFillColor,
                                  kGameHudTextOutlineColor,
                                  m_ScoreLabelOutline,
                                  m_ScoreLabel);
        for (const auto &outline : m_ScoreLabelOutline)
        {
            AddElements(outline);
        }
        AddElements(m_ScoreLabel);
    }

    if (!m_ScoreValue)
    {
        CreateOutlinedTextObjects("0",
                                  kGameHudScoreValueSize,
                                  {0.0f, 0.0f},
                                  95.0f,
                                  kGameHudTextFillColor,
                                  kGameHudTextOutlineColor,
                                  m_ScoreValueOutline,
                                  m_ScoreValue,
                                  &m_ScoreValueDrawable,
                                  &m_ScoreValueOutlineDrawables);
        for (const auto &outline : m_ScoreValueOutline)
        {
            AddElements(outline);
        }
        AddElements(m_ScoreValue);
    }

    if (!m_HighScoreLabel)
    {
        CreateOutlinedTextObjects("HIGHSCORE",
                                  kGameHudHighScoreLabelSize,
                                  {0.0f, 0.0f},
                                  95.0f,
                                  kGameHudTextFillColor,
                                  kGameHudTextOutlineColor,
                                  m_HighScoreLabelOutline,
                                  m_HighScoreLabel);
        for (const auto &outline : m_HighScoreLabelOutline)
        {
            AddElements(outline);
        }
        AddElements(m_HighScoreLabel);
    }

    if (!m_HighScoreValue)
    {
        CreateOutlinedTextObjects("0",
                                  kGameHudHighScoreValueSize,
                                  {0.0f, 0.0f},
                                  95.0f,
                                  kGameHudTextFillColor,
                                  kGameHudTextOutlineColor,
                                  m_HighScoreValueOutline,
                                  m_HighScoreValue,
                                  &m_HighScoreValueDrawable,
                                  &m_HighScoreValueOutlineDrawables);
        for (const auto &outline : m_HighScoreValueOutline)
        {
            AddElements(outline);
        }
        AddElements(m_HighScoreValue);
    }

    m_LeftTopButton093 = std::make_shared<Button>(Resource::Game_Button_093);
    m_LeftTopButton093->SetZIndex(95.0f);
    m_LeftTopButton093->SetScale({kGameHudButtonScale, kGameHudButtonScale});
    m_LeftTopButton093->SetVisible(true);
    m_LeftTopButton093->SetSFX(Resource::SETTING_SFX);
    m_LeftTopButton093->SetOnClickFunction([this]()
                                           { TogglePauseMenu(); });
    AddElements(m_LeftTopButton093);

    m_LeftTopButton031 = std::make_shared<Button>(Resource::Game_Button_031);
    m_LeftTopButton031->SetZIndex(95.0f);
    m_LeftTopButton031->SetScale({kGameHudButtonScale, kGameHudButtonScale});
    m_LeftTopButton031->SetVisible(true);
    m_LeftTopButton031->SetOnClickFunction([this]()
                                           {
                                               SetPauseMenuVisible(false);
                                               if (m_OnRestartLevel)
                                               {
                                                   m_OnRestartLevel();
                                               } });
    AddElements(m_LeftTopButton031);

    m_PauseMenuBackdrop = std::make_shared<Util::GameObject>(
        std::make_shared<Util::DebugBox>(glm::vec4{0.0f, 0.0f, 0.0f, 0.58f}, 1.0f), 90.0f);
    m_PauseMenuBackdrop->SetVisible(false);
    AddElements(m_PauseMenuBackdrop);

    m_PauseMenu069 = std::make_shared<Button>(Resource::Game_Menu_Item_069);
    m_PauseMenu069->SetZIndex(96.0f);
    m_PauseMenu069->SetScale({kGamePauseMenu069Scale, kGamePauseMenu069Scale});
    m_PauseMenu069->SetVisible(false);
    m_PauseMenu069->SetSFX(Resource::SETTING_SFX);
    m_PauseMenu069->SetOnClickFunction([this]()
                                       { SetPauseMenuVisible(false); });
    AddElements(m_PauseMenu069);

    m_PauseMenu082 = std::make_shared<Button>(Resource::Game_Menu_Item_082);
    m_PauseMenu082->SetZIndex(96.0f);
    m_PauseMenu082->SetScale({kGamePauseMenu082Scale, kGamePauseMenu082Scale});
    m_PauseMenu082->SetVisible(false);
    m_PauseMenu082->SetSFX(Resource::SETTING_SFX);
    m_PauseMenu082->SetOnClickFunction([this]()
                                       {
                                           SetPauseMenuVisible(false);
                                           if (m_OnRestartLevel)
                                           {
                                               m_OnRestartLevel();
                                           } });
    AddElements(m_PauseMenu082);

    m_PauseMenu073 = std::make_shared<Button>(Resource::Game_Menu_Item_073);
    m_PauseMenu073->SetZIndex(96.0f);
    m_PauseMenu073->SetScale({kGamePauseMenu073Scale, kGamePauseMenu073Scale});
    m_PauseMenu073->SetVisible(false);
    m_PauseMenu073->SetSFX(Resource::SETTING_SFX);
    m_PauseMenu073->SetOnClickFunction([this]()
                                       {
                                           SetPauseMenuVisible(false);
                                           if (m_OnOpenLevelSelect)
                                           {
                                               m_OnOpenLevelSelect();
                                           } });
    AddElements(m_PauseMenu073);

    m_PauseMenu005 = std::make_shared<Button>(Resource::Game_Menu_Item_005);
    m_PauseMenu005->SetZIndex(96.0f);
    m_PauseMenu005->SetScale({kGamePauseMenu005Scale, kGamePauseMenu005Scale});
    m_PauseMenu005->SetVisible(false);
    m_PauseMenu005->SetSFX(Resource::SETTING_SFX);
    m_PauseMenu005->SetOnClickFunction([this]()
                                       { ToggleMusicMute(); });
    AddElements(m_PauseMenu005);

    m_PauseMenu040Overlay = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Game_Menu_Item_040), 97.0f);
    m_PauseMenu040Overlay->m_Transform.scale = {kGamePauseMenu040Scale, kGamePauseMenu040Scale};
    m_PauseMenu040Overlay->SetVisible(false);
    AddElements(m_PauseMenu040Overlay);

    m_PauseMenu063 = std::make_shared<Button>(Resource::Game_Menu_Item_063);
    m_PauseMenu063->SetZIndex(96.0f);
    m_PauseMenu063->SetScale({kGamePauseMenu063Scale, kGamePauseMenu063Scale});
    m_PauseMenu063->SetVisible(false);
    AddElements(m_PauseMenu063);

    m_PauseMenuLevelTitle = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Level_Title_1_1), 96.0f);
    m_PauseMenuLevelTitle->m_Transform.scale = {kGamePauseMenuLevelTitleScale, kGamePauseMenuLevelTitleScale};
    m_PauseMenuLevelTitle->SetVisible(false);
    AddElements(m_PauseMenuLevelTitle);

    // Build Level Clear Screen UI
    m_LevelClearBackdrop = std::make_shared<Util::GameObject>(
        std::make_shared<Util::DebugBox>(glm::vec4{0.0f, 0.0f, 0.0f, 0.75f}, 1.0f), 90.0f);
    m_LevelClearBackdrop->SetVisible(false);
    AddElements(m_LevelClearBackdrop);

    m_LevelClearTitle = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Text>(kUIFont, kLevelClearTitleSize, "LEVEL CLEARED!", kGameHudTextFillColor), 98.0f);
    m_LevelClearTitle->SetVisible(false);
    AddElements(m_LevelClearTitle);

    // Create star rating display
    for (int i = 0; i < 3; ++i)
    {
        auto emptyStarDrawable = std::make_shared<Util::Image>(Resource::Star_Empty);
        emptyStarDrawable->SetOpacity(kLevelClearEmptyStarOpacity);
        m_LevelClearStars[i] = std::make_shared<Util::GameObject>(
            emptyStarDrawable, 97.0f);
        m_LevelClearStars[i]->m_Transform.scale = {kLevelClearStarScale, kLevelClearStarScale};
        m_LevelClearStars[i]->SetVisible(false);
        AddElements(m_LevelClearStars[i]);

        m_LevelClearEarnedStars[i] = std::make_shared<Util::GameObject>(
            std::make_shared<Util::Image>(Resource::Star_Filled), 97.2f);
        m_LevelClearEarnedStars[i]->m_Transform.scale = {kLevelClearStarScale, kLevelClearStarScale};
        m_LevelClearEarnedStars[i]->SetVisible(false);
        AddElements(m_LevelClearEarnedStars[i]);
    }

    // Level Clear Score
    CreateOutlinedTextObjects(FormatScore(0),
                              kLevelClearScoreValueSize,
                              {0.0f, 0.0f},
                              97.0f,
                              kLevelClearScoreFillColor,
                              kLevelClearScoreOutlineColor,
                              m_LevelClearScoreOutline,
                              m_LevelClearScore,
                              &m_LevelClearScoreDrawable,
                              &m_LevelClearScoreOutlineDrawables);
    for (const auto &outline : m_LevelClearScoreOutline)
    {
        outline->SetVisible(false);
        AddElements(outline);
    }
    m_LevelClearScore->SetVisible(false);
    AddElements(m_LevelClearScore);

    // Level Clear High Score
    CreateOutlinedTextObjects("BEST " + FormatScore(0),
                              kLevelClearHighScoreValueSize,
                              {0.0f, 0.0f},
                              97.0f,
                              kGameHudTextFillColor,
                              kGameHudTextOutlineColor,
                              m_LevelClearHighScoreOutline,
                              m_LevelClearHighScore,
                              &m_LevelClearHighScoreDrawable,
                              &m_LevelClearHighScoreOutlineDrawables);
    for (const auto &outline : m_LevelClearHighScoreOutline)
    {
        outline->SetVisible(false);
        AddElements(outline);
    }
    m_LevelClearHighScore->SetVisible(false);
    AddElements(m_LevelClearHighScore);

    for (int i = 0; i < 3; ++i)
    {
        m_LevelClearBestStars[i] = std::make_shared<Util::GameObject>(
            std::make_shared<Util::Image>(Resource::Star_Empty), 97.5f);
        m_LevelClearBestStars[i]->SetVisible(false);
        AddElements(m_LevelClearBestStars[i]);
    }

    // Level Clear Buttons
    m_LevelClearMenuButton = std::make_shared<Button>(Resource::Game_Menu_Item_073);
    m_LevelClearMenuButton->SetZIndex(98.0f);
    m_LevelClearMenuButton->SetScale({kLevelClearButtonScale, kLevelClearButtonScale});
    m_LevelClearMenuButton->SetVisible(false);
    m_LevelClearMenuButton->SetSFX(Resource::SETTING_SFX);
    m_LevelClearMenuButton->SetOnClickFunction([this]()
                                               {
                                                   if (m_OnOpenLevelSelect)
                                                   {
                                                       m_OnOpenLevelSelect();
                                                   }
                                               });
    AddElements(m_LevelClearMenuButton);

    m_LevelClearRestartButton = std::make_shared<Button>(Resource::Game_Menu_Item_082);
    m_LevelClearRestartButton->SetZIndex(98.0f);
    m_LevelClearRestartButton->SetScale({kLevelClearButtonScale, kLevelClearButtonScale});
    m_LevelClearRestartButton->SetVisible(false);
    m_LevelClearRestartButton->SetSFX(Resource::SETTING_SFX);
    m_LevelClearRestartButton->SetOnClickFunction([this]()
                                                  {
                                                      if (m_OnRestartLevel)
                                                      {
                                                          m_OnRestartLevel();
                                                      }
                                                  });
    AddElements(m_LevelClearRestartButton);

    m_LevelClearNextButton = std::make_shared<Button>(Resource::Game_Menu_Item_078);
    m_LevelClearNextButton->SetZIndex(98.0f);
    m_LevelClearNextButton->SetScale({kLevelClearButtonScale, kLevelClearButtonScale});
    m_LevelClearNextButton->SetVisible(false);
    m_LevelClearNextButton->SetSFX(Resource::SETTING_SFX);
    m_LevelClearNextButton->SetOnClickFunction([this]()
                                               {
                                                   if (m_OnNextLevel)
                                                   {
                                                       m_OnNextLevel();
                                                   }
                                               });
    AddElements(m_LevelClearNextButton);
    
    m_LevelFailedBackdrop = std::make_shared<Util::GameObject>(
        std::make_shared<Util::DebugBox>(glm::vec4{0.0f, 0.0f, 0.0f, 0.75f}, 1.0f), 90.0f);
    m_LevelFailedBackdrop->SetVisible(false);
    AddElements(m_LevelFailedBackdrop);

    m_LevelFailedTitle = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Text>(kUIFont, kLevelFailedTitleSize, "LEVEL FAILED!", kGameHudTextFillColor), 98.0f);
    m_LevelFailedTitle->SetVisible(false);
    AddElements(m_LevelFailedTitle);

    m_LevelFailedPig = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::PIG_SMALL), 98.0f);
    m_LevelFailedPig->m_Transform.scale = {kLevelFailedPigScale, kLevelFailedPigScale};
    m_LevelFailedPig->SetVisible(false);
    AddElements(m_LevelFailedPig);

    m_LevelFailedMenuButton = std::make_shared<Button>(Resource::Game_Menu_Item_073);
    m_LevelFailedMenuButton->SetZIndex(98.0f);
    m_LevelFailedMenuButton->SetScale({kLevelFailedButtonScale, kLevelFailedButtonScale});
    m_LevelFailedMenuButton->SetVisible(false);
    m_LevelFailedMenuButton->SetSFX(Resource::SETTING_SFX);
    m_LevelFailedMenuButton->SetOnClickFunction([this]()
                                                {
                                                    if (m_OnOpenLevelSelect)
                                                    {
                                                        m_OnOpenLevelSelect();
                                                    }
                                                });
    AddElements(m_LevelFailedMenuButton);

    m_LevelFailedRestartButton = std::make_shared<Button>(Resource::Game_Menu_Item_082);
    m_LevelFailedRestartButton->SetZIndex(98.0f);
    m_LevelFailedRestartButton->SetScale({kLevelFailedButtonScale, kLevelFailedButtonScale});
    m_LevelFailedRestartButton->SetVisible(false);
    m_LevelFailedRestartButton->SetSFX(Resource::SETTING_SFX);
    m_LevelFailedRestartButton->SetOnClickFunction([this]()
                                                   {
                                                       if (m_OnRestartLevel)
                                                       {
                                                           m_OnRestartLevel();
                                                       }
                                                   });
    AddElements(m_LevelFailedRestartButton);

    m_LevelFailedNextButton = std::make_shared<Button>(Resource::Game_Menu_Item_078);
    m_LevelFailedNextButton->SetZIndex(98.0f);
    m_LevelFailedNextButton->SetScale({kLevelFailedButtonScale, kLevelFailedButtonScale});
    m_LevelFailedNextButton->SetVisible(false);
    m_LevelFailedNextButton->SetSFX(Resource::SETTING_SFX);
    m_LevelFailedNextButton->SetOnClickFunction([this]()
                                                {
                                                    if (m_OnNextLevel)
                                                    {
                                                        m_OnNextLevel();
                                                    }
                                                });
    AddElements(m_LevelFailedNextButton);

    SetPauseMenuVisible(false);
}

void GameScene::UpdateHudPositions()
{
    if (!m_LeftTopButton093 && !m_LeftTopButton031 && !m_ScoreLabel && !m_ScoreValue)
    {
        return;
    }

    const glm::vec2 cameraPos = Util::GetCameraPosition();
    const glm::vec2 viewportSize = Util::GetViewportSize();
    const float zoom = Util::GetCameraZoom();
    const glm::vec2 topLeftAnchor = cameraPos +
                                    glm::vec2{
                                        -viewportSize.x * 0.5f / zoom + kGameHudLeftPadding / zoom,
                                        viewportSize.y * 0.5f / zoom - kGameHudTopPadding / zoom};

    if (m_LeftTopButton093)
    {
        m_LeftTopButton093->SetPosition(topLeftAnchor);
    }

    if (m_LeftTopButton031)
    {
        m_LeftTopButton031->SetPosition(topLeftAnchor + glm::vec2{kGameHudButtonSpacing / zoom, 0.0f});
    }

    // High score positioned at top right
    const glm::vec2 topRightAnchor = cameraPos +
                                     glm::vec2{
                                         viewportSize.x * 0.5f / zoom - kGameHudLeftPadding / zoom,
                                         viewportSize.y * 0.5f / zoom - kGameHudTopPadding / zoom};

    const glm::vec2 scoreAnchor = topRightAnchor + glm::vec2{kGameHudScoreOffsetX / zoom, 0.0f};
    PositionOutlinedTextObjects(scoreAnchor + glm::vec2{0.0f, kGameHudScoreLabelOffsetY / zoom},
                                m_ScoreLabelOutline,
                                m_ScoreLabel,
                                zoom);
    PositionOutlinedTextObjects(scoreAnchor + glm::vec2{0.0f, kGameHudScoreValueOffsetY / zoom},
                                m_ScoreValueOutline,
                                m_ScoreValue,
                                zoom);

    const glm::vec2 highScoreAnchor = topRightAnchor + glm::vec2{kGameHudHighScoreOffsetX / zoom, 0.0f};
    PositionOutlinedTextObjects(highScoreAnchor + glm::vec2{0.0f, kGameHudHighScoreLabelOffsetY / zoom},
                                m_HighScoreLabelOutline,
                                m_HighScoreLabel,
                                zoom);
    PositionOutlinedTextObjects(highScoreAnchor + glm::vec2{0.0f, kGameHudHighScoreValueOffsetY / zoom},
                                m_HighScoreValueOutline,
                                m_HighScoreValue,
                                zoom);

    if (m_PauseMenu069)
    {
        m_PauseMenu069->SetPosition(topLeftAnchor +
                                    glm::vec2{
                                        kGamePauseMenu069OffsetX / zoom,
                                        -kGamePauseMenu069OffsetY / zoom});
    }

    if (m_PauseMenu082)
    {
        m_PauseMenu082->SetPosition(topLeftAnchor +
                                    glm::vec2{
                                        kGamePauseMenu082OffsetX / zoom,
                                        -kGamePauseMenu082OffsetY / zoom});
    }

    if (m_PauseMenu073)
    {
        m_PauseMenu073->SetPosition(topLeftAnchor +
                                    glm::vec2{
                                        kGamePauseMenu073OffsetX / zoom,
                                        -kGamePauseMenu073OffsetY / zoom});
    }

    if (m_PauseMenu005)
    {
        m_PauseMenu005->SetPosition(topLeftAnchor +
                                    glm::vec2{
                                        kGamePauseMenu005OffsetX / zoom,
                                        -kGamePauseMenu005OffsetY / zoom});
    }

    if (m_PauseMenu040Overlay)
    {
        m_PauseMenu040Overlay->m_Transform.translation = topLeftAnchor +
                                                         glm::vec2{
                                                             kGamePauseMenu040OffsetX / zoom,
                                                             -kGamePauseMenu040OffsetY / zoom};
        m_PauseMenu040Overlay->m_Transform.scale = {
            kGamePauseMenu040Scale,
            kGamePauseMenu040Scale};
    }

    if (m_PauseMenu063)
    {
        m_PauseMenu063->SetPosition(topLeftAnchor +
                                    glm::vec2{
                                        kGamePauseMenu063OffsetX / zoom,
                                        -kGamePauseMenu063OffsetY / zoom});
    }

    if (m_PauseMenuBackdrop)
    {
        m_PauseMenuBackdrop->m_Transform.translation = cameraPos +
                                                       glm::vec2{
                                                           -viewportSize.x * 0.5f / zoom + kGamePauseMenuBackdropCenterOffsetX / zoom,
                                                           kGamePauseMenuBackdropCenterOffsetY / zoom};
        m_PauseMenuBackdrop->m_Transform.scale = {
            kGamePauseMenuBackdropWidth / zoom,
            viewportSize.y * kGamePauseMenuBackdropHeightRatio / zoom};
    }

    if (m_PauseMenuLevelTitle)
    {
        m_PauseMenuLevelTitle->m_Transform.translation = cameraPos +
                                                         glm::vec2{
                                                             -viewportSize.x * 0.5f / zoom + kGamePauseMenuLevelTitleOffsetX / zoom,
                                                             kGamePauseMenuLevelTitleOffsetY / zoom};
        m_PauseMenuLevelTitle->m_Transform.scale = {kGamePauseMenuLevelTitleScale, kGamePauseMenuLevelTitleScale};
    }
}

void GameScene::SetPauseMenuVisible(const bool visible)
{
    m_IsPauseMenuVisible = visible;

    if (m_LeftTopButton093)
    {
        m_LeftTopButton093->SetVisible(!visible);
    }
    if (m_LeftTopButton031)
    {
        m_LeftTopButton031->SetVisible(!visible);
    }

    if (m_PauseMenuBackdrop)
    {
        m_PauseMenuBackdrop->SetVisible(visible);
    }
    if (m_PauseMenu069)
    {
        m_PauseMenu069->SetVisible(visible);
    }
    if (m_PauseMenu082)
    {
        m_PauseMenu082->SetVisible(visible);
    }
    if (m_PauseMenu073)
    {
        m_PauseMenu073->SetVisible(visible);
    }
    if (m_PauseMenu005)
    {
        m_PauseMenu005->SetVisible(visible);
    }
    if (m_PauseMenu040Overlay)
    {
        m_PauseMenu040Overlay->SetVisible(visible && m_IsMusicMuted);
    }
    if (m_PauseMenu063)
    {
        m_PauseMenu063->SetVisible(visible);
    }

    if (m_PauseMenuLevelTitle)
    {
        m_PauseMenuLevelTitle->SetVisible(visible);
        if (visible && m_LevelManager)
        {
            int levelNum = m_LevelManager->GetLevel();
            std::string levelTitleResource;

            if (levelNum == 1)
                levelTitleResource = Resource::Level_Title_1_1;
            else if (levelNum == 2)
                levelTitleResource = Resource::Level_Title_1_2;
            else if (levelNum == 3)
                levelTitleResource = Resource::Level_Title_1_3;
            else if (levelNum == 4)
                levelTitleResource = Resource::Level_Title_1_4;
            else if (levelNum == 5)
                levelTitleResource = Resource::Level_Title_1_5;
            else if (levelNum == 6)
                levelTitleResource = Resource::Level_Title_1_6;
            else if (levelNum == 7)
                levelTitleResource = Resource::Level_Title_1_7;
            else if (levelNum == 8)
                levelTitleResource = Resource::Level_Title_1_8;
            else if (levelNum == 9)
                levelTitleResource = Resource::Level_Title_1_9;
            else if (levelNum == 10)
                levelTitleResource = Resource::Level_Title_1_10;

            if (!levelTitleResource.empty())
            {
                m_PauseMenuLevelTitle->SetDrawable(std::make_shared<Util::Image>(levelTitleResource));
            }
        }
    }

    if (visible)
    {
        m_PauseMenuInputBlockedUntilRelease = true;
        SetPauseMenuButtonsInputEnabled(false);
    }
    else
    {
        m_PauseMenuInputBlockedUntilRelease = false;
        SetPauseMenuButtonsInputEnabled(true);
    }
}

void GameScene::UpdateScoreHud()
{
    const std::string scoreText = FormatScore(m_ScoringSystem.GetScore());
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

    const std::string highScoreText = FormatScore(m_HudHighScore);
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

void GameScene::ResetScoreState()
{
    m_ScoringSystem.Reset();
    m_RemainingPigCount = 0;
    m_LevelCleared = false;
    m_LevelFailed = false;
    m_LeftoverBirdsAwarded = false;
    m_LevelClearAnimationTime = 0.0f;
    m_IsLevelClearScreenVisible = false;
    m_IsLevelFailedScreenVisible = false;

    if (m_LevelClearBackdrop)
    {
        m_LevelClearBackdrop->SetVisible(false);
    }
    if (m_LevelClearTitle)
    {
        m_LevelClearTitle->SetVisible(false);
    }
    for (const auto &star : m_LevelClearStars)
    {
        if (star)
        {
            star->SetVisible(false);
        }
    }
    for (const auto &star : m_LevelClearEarnedStars)
    {
        if (star)
        {
            star->SetVisible(false);
        }
    }
    for (const auto &outline : m_LevelClearScoreOutline)
    {
        if (outline)
        {
            outline->SetVisible(false);
        }
    }
    if (m_LevelClearScore)
    {
        m_LevelClearScore->SetVisible(false);
    }
    for (const auto &outline : m_LevelClearHighScoreOutline)
    {
        if (outline)
        {
            outline->SetVisible(false);
        }
    }
    if (m_LevelClearHighScore)
    {
        m_LevelClearHighScore->SetVisible(false);
    }
    for (const auto &star : m_LevelClearBestStars)
    {
        if (star)
        {
            star->SetVisible(false);
        }
    }
    if (m_LevelClearMenuButton)
    {
        m_LevelClearMenuButton->SetVisible(false);
    }
    if (m_LevelClearRestartButton)
    {
        m_LevelClearRestartButton->SetVisible(false);
    }
    if (m_LevelClearNextButton)
    {
        m_LevelClearNextButton->SetVisible(false);
    }

    if (m_LevelFailedBackdrop)
    {
        m_LevelFailedBackdrop->SetVisible(false);
    }
    if (m_LevelFailedTitle)
    {
        m_LevelFailedTitle->SetVisible(false);
    }
    if (m_LevelFailedPig)
    {
        m_LevelFailedPig->SetVisible(false);
    }
    if (m_LevelFailedMenuButton)
    {
        m_LevelFailedMenuButton->SetVisible(false);
    }
    if (m_LevelFailedRestartButton)
    {
        m_LevelFailedRestartButton->SetVisible(false);
    }
    if (m_LevelFailedNextButton)
    {
        m_LevelFailedNextButton->SetVisible(false);
    }

    for (const auto &object : m_LevelManager->GetGameObjects())
    {
        if (!object)
        {
            continue;
        }

        object->SetDestroyed(false);
        object->ResetHealth();
        object->SetVisible(true);
        // Only set physics participation for objects that should participate
        // DECOR (Unknown kind) and Slingshot should not participate in physics
        if (object->GetEntityKind() == Character::EntityKind::Environment ||
            object->GetEntityKind() == Character::EntityKind::Pig)
        {
            object->SetParticipatesInPhysics(true);
        }

        if (object->GetEntityKind() == Character::EntityKind::Pig)
        {
            ++m_RemainingPigCount;
        }

        if (object->GetEntityKind() == Character::EntityKind::Environment && !object->IsSpecialItem())
        {
            object->SetScoreBudgetRemaining(m_ScoringSystem.GetDamageBudget(object->GetMaterialType()));
            object->SetOnDamageCallback([this](Character *c, float appliedDamage) {
                const float damageRatio = appliedDamage / std::max(0.0001f, c->GetMaxHealth());
                const int estimated = std::max(
                    10,
                    static_cast<int>(std::lround(static_cast<float>(m_ScoringSystem.GetDamageBudget(c->GetMaterialType())) * std::clamp(damageRatio, 0.0f, 1.0f))));
                const int awarded = c->DrainScoreBudget(estimated);
                if (awarded > 0)
                {
                    m_ScoringSystem.AwardBlockDamage(c->GetMaterialType(), damageRatio, awarded);
                    SpawnFloatingScore(c->GetPosition(), awarded, Util::Color::FromRGB(255, 255, 255));
                    UpdateScoreHud();
                }
            });
        }
        else
        {
            object->SetScoreBudgetRemaining(0);
            object->SetOnDamageCallback(nullptr);
        }
    }
}

void GameScene::RefreshRemainingPigCount()
{
    if (!m_LevelManager)
    {
        m_RemainingPigCount = 0;
        return;
    }

    int alivePigCount = 0;
    for (const auto &object : m_LevelManager->GetGameObjects())
    {
        if (!object)
        {
            continue;
        }

        if (object->GetEntityKind() != Character::EntityKind::Pig)
        {
            continue;
        }

        if (object->GetHealth() > 0.0f && !object->IsDestroyed())
        {
            ++alivePigCount;
        }
    }

    m_RemainingPigCount = alivePigCount;
    if (m_RemainingPigCount == 0)
    {
        m_LevelCleared = true;
        m_LevelFailed = false;
    }
}

void GameScene::UpdateWinState()
{
    if (!m_LevelCleared || m_LevelFailed)
    {
        return;
    }

    if (!m_IsLevelClearScreenVisible)
    {
        if (!m_LeftoverBirdsAwarded && m_BirdLaunchController)
        {
            m_ScoringSystem.AwardLeftoverBirds(m_BirdLaunchController->GetRemainingBirdCountForBonus());
            m_LeftoverBirdsAwarded = true;
            UpdateScoreHud();
        }

        m_ScoringSystem.CommitCurrentScoreToHighScore();
        m_HudHighScore = m_ScoringSystem.GetHighScore();
        PersistLevelHighScore();
        m_IsLevelClearScreenVisible = true;
        m_LevelClearAnimationTime = 0.0f;
        SetPauseMenuVisible(false);
    }

    m_LevelClearAnimationTime += std::max(0.0f, Util::Time::GetDeltaTimeMs() / 1000.0f);

    // Hide game HUD buttons
    if (m_LeftTopButton093)
    {
        m_LeftTopButton093->SetVisible(false);
    }
    if (m_LeftTopButton031)
    {
        m_LeftTopButton031->SetVisible(false);
    }
    const glm::vec2 cameraPos = Util::GetCameraPosition();
    const glm::vec2 viewportSize = Util::GetViewportSize();
    const float zoom = Util::GetCameraZoom();
    const int score = m_ScoringSystem.GetScore();
    const int stars = ComputeStarCount(score);
    const int highScore = m_ScoringSystem.GetHighScore();
    const int highScoreStars = ComputeStarCount(highScore);
    if (m_LevelClearBackdrop)
    {
        m_LevelClearBackdrop->SetVisible(true);
        m_LevelClearBackdrop->m_Transform.translation = cameraPos;
        m_LevelClearBackdrop->m_Transform.scale = {
            kLevelClearPanelWidth / zoom,
            viewportSize.y * kLevelClearPanelHeightRatio / zoom};
    }
    if (m_LevelClearTitle)
    {
        m_LevelClearTitle->SetVisible(true);
        m_LevelClearTitle->m_Transform.translation = cameraPos + glm::vec2{0.0f, kLevelClearTitleOffsetY / zoom};
    }
    const float starStartX = -(kLevelClearStarSpacing / zoom);
    const float baseStarScale = kLevelClearStarScale / zoom;
    for (int i = 0; i < 3; ++i)
    {
        if (!m_LevelClearStars[i])
        {
            continue;
        }
        const glm::vec2 baseStarPosition = cameraPos + glm::vec2{
            starStartX + static_cast<float>(i) * (kLevelClearStarSpacing / zoom),
            kLevelClearStarsOffsetY / zoom};
        const bool earnedStar = i < stars;
        m_LevelClearStars[i]->SetVisible(true);
        m_LevelClearStars[i]->m_Transform.scale = {baseStarScale, baseStarScale};
        m_LevelClearStars[i]->m_Transform.translation = baseStarPosition;

        if (!m_LevelClearEarnedStars[i])
        {
            continue;
        }

        if (earnedStar)
        {
            const float starElapsedTime = m_LevelClearAnimationTime - static_cast<float>(i) * kLevelClearStarPopDelay;
            const float popScale = ComputeStarPopScale(starElapsedTime);
            if (popScale > 0.0f)
            {
                const float normalizedProgress = std::clamp(starElapsedTime / kLevelClearStarPopDuration, 0.0f, 1.0f);
                const float yOffset = (1.0f - normalizedProgress) * (kLevelClearStarPopYOffset / zoom);
                m_LevelClearEarnedStars[i]->SetVisible(true);
                m_LevelClearEarnedStars[i]->m_Transform.scale = {
                    baseStarScale * popScale,
                    baseStarScale * popScale};
                m_LevelClearEarnedStars[i]->m_Transform.translation = baseStarPosition + glm::vec2{0.0f, yOffset};
            }
            else
            {
                m_LevelClearEarnedStars[i]->SetVisible(false);
                m_LevelClearEarnedStars[i]->m_Transform.scale = {0.0f, 0.0f};
                m_LevelClearEarnedStars[i]->m_Transform.translation = baseStarPosition;
            }
        }
        else
        {
            m_LevelClearEarnedStars[i]->SetVisible(false);
            m_LevelClearEarnedStars[i]->m_Transform.scale = {0.0f, 0.0f};
            m_LevelClearEarnedStars[i]->m_Transform.translation = baseStarPosition;
        }
    }
    if (m_LevelClearScore && m_LevelClearScoreDrawable)
    {
        m_LevelClearScore->SetVisible(true);
        const std::string scoreStr = FormatScore(score);
        m_LevelClearScoreDrawable->SetText(scoreStr);
        UpdateOutlineDrawables(&m_LevelClearScoreDrawable, &m_LevelClearScoreOutlineDrawables, scoreStr);
        for (const auto &outline : m_LevelClearScoreOutline)
        {
            outline->SetVisible(true);
        }
        m_LevelClearScore->m_Transform.translation = cameraPos + glm::vec2{0.0f, kLevelClearScoreOffsetY / zoom};
        for (size_t i = 0; i < m_LevelClearScoreOutline.size(); ++i)
        {
            if (m_LevelClearScoreOutline[i])
            {
                m_LevelClearScoreOutline[i]->m_Transform.translation =
                    m_LevelClearScore->m_Transform.translation + kGameHudOutlineOffsets[i] / zoom;
            }
        }
    }
    if (m_LevelClearHighScore && m_LevelClearHighScoreDrawable)
    {
        m_LevelClearHighScore->SetVisible(true);
        const std::string highScoreText = "BEST " + FormatScore(highScore);
        m_LevelClearHighScoreDrawable->SetText(highScoreText);
        UpdateOutlineDrawables(&m_LevelClearHighScoreDrawable, &m_LevelClearHighScoreOutlineDrawables, highScoreText);
        for (const auto &outline : m_LevelClearHighScoreOutline)
        {
            outline->SetVisible(true);
        }
        m_LevelClearHighScore->m_Transform.translation = cameraPos + glm::vec2{0.0f, kLevelClearHighScoreOffsetY / zoom};
        for (size_t i = 0; i < m_LevelClearHighScoreOutline.size(); ++i)
        {
            if (m_LevelClearHighScoreOutline[i])
            {
                m_LevelClearHighScoreOutline[i]->m_Transform.translation =
                    m_LevelClearHighScore->m_Transform.translation + kGameHudOutlineOffsets[i] / zoom;
            }
        }
    }
    for (int i = 0; i < 3; ++i)
    {
        if (!m_LevelClearBestStars[i])
        {
            continue;
        }
        m_LevelClearBestStars[i]->SetVisible(true);
        m_LevelClearBestStars[i]->m_Transform.scale = {kLevelClearBestStarScale / zoom, kLevelClearBestStarScale / zoom};
        m_LevelClearBestStars[i]->m_Transform.translation = cameraPos + glm::vec2{
            kLevelClearBestStarsOffsetX / zoom + static_cast<float>(i) * (kLevelClearBestStarSpacing / zoom),
            kLevelClearHighScoreOffsetY / zoom};
        m_LevelClearBestStars[i]->SetDrawable(std::make_shared<Util::Image>(
            i < highScoreStars ? Resource::Star_Filled : Resource::Star_Empty));
    }
    if (m_LevelClearMenuButton)
    {
        m_LevelClearMenuButton->SetVisible(true);
        m_LevelClearMenuButton->SetPosition(cameraPos + glm::vec2{
            -kLevelClearButtonSpacing / zoom,
            kLevelClearButtonBaseOffsetY / zoom});
    }
    if (m_LevelClearRestartButton)
    {
        m_LevelClearRestartButton->SetVisible(true);
        m_LevelClearRestartButton->SetPosition(cameraPos + glm::vec2{
            0.0f,
            kLevelClearButtonBaseOffsetY / zoom});
    }
    if (m_LevelClearNextButton)
    {
        m_LevelClearNextButton->SetVisible(true);
        m_LevelClearNextButton->SetPosition(cameraPos + glm::vec2{
            kLevelClearButtonSpacing / zoom,
            kLevelClearButtonBaseOffsetY / zoom});
    }
}

void GameScene::UpdateFailState()
{
    if (!m_LevelFailed)
    {
        const bool hasFailedByExhaustingBirds =
            m_BirdLaunchController &&
            !m_LevelCleared &&
            m_RemainingPigCount > 0 &&
            m_BirdLaunchController->IsOutOfBirds();

        if (!hasFailedByExhaustingBirds)
        {
            return;
        }

        m_LevelFailed = true;
    }

    if (m_IsLevelFailedScreenVisible || m_IsLevelClearScreenVisible)
    {
        return;
    }

    m_IsLevelFailedScreenVisible = true;
    SetPauseMenuVisible(false);

    if (m_LeftTopButton093)
    {
        m_LeftTopButton093->SetVisible(false);
    }
    if (m_LeftTopButton031)
    {
        m_LeftTopButton031->SetVisible(false);
    }

    const glm::vec2 cameraPos = Util::GetCameraPosition();
    const glm::vec2 viewportSize = Util::GetViewportSize();
    const float zoom = Util::GetCameraZoom();

    if (m_LevelFailedBackdrop)
    {
        m_LevelFailedBackdrop->SetVisible(true);
        m_LevelFailedBackdrop->m_Transform.translation = cameraPos;
        m_LevelFailedBackdrop->m_Transform.scale = {
            kLevelClearPanelWidth / zoom,
            viewportSize.y * kLevelClearPanelHeightRatio / zoom};
    }

    if (m_LevelFailedTitle)
    {
        m_LevelFailedTitle->SetVisible(true);
        m_LevelFailedTitle->m_Transform.translation = cameraPos + glm::vec2{0.0f, kLevelFailedTitleOffsetY / zoom};
    }

    if (m_LevelFailedPig)
    {
        m_LevelFailedPig->SetVisible(true);
        m_LevelFailedPig->m_Transform.translation = cameraPos + glm::vec2{0.0f, kLevelFailedPigOffsetY / zoom};
        m_LevelFailedPig->m_Transform.scale = {kLevelFailedPigScale / zoom, kLevelFailedPigScale / zoom};
    }

    if (m_LevelFailedMenuButton)
    {
        m_LevelFailedMenuButton->SetVisible(true);
        m_LevelFailedMenuButton->SetPosition(cameraPos + glm::vec2{
            -kLevelFailedButtonSpacing / zoom,
            kLevelFailedButtonBaseOffsetY / zoom});
    }

    if (m_LevelFailedRestartButton)
    {
        m_LevelFailedRestartButton->SetVisible(true);
        m_LevelFailedRestartButton->SetPosition(cameraPos + glm::vec2{
            0.0f,
            kLevelFailedButtonBaseOffsetY / zoom});
    }

    if (m_LevelFailedNextButton)
    {
        m_LevelFailedNextButton->SetVisible(true);
        m_LevelFailedNextButton->SetPosition(cameraPos + glm::vec2{
            kLevelFailedButtonSpacing / zoom,
            kLevelFailedButtonBaseOffsetY / zoom});
    }
}

void GameScene::SpawnOutlinedFloatingScore(const glm::vec2 &position,
                                           const std::string &text,
                                           const Util::Color &frontColor)
{
    const Util::Color outlineColor = Util::Color::FromRGB(170, 110, 80, 255);
    const std::array<glm::vec2, 4> offsets = {
        glm::vec2{-2.0f, 0.0f},
        glm::vec2{2.0f, 0.0f},
        glm::vec2{0.0f, -2.0f},
        glm::vec2{0.0f, 2.0f}};

    for (const auto &offset : offsets)
    {
        auto shadowDrawable = std::make_shared<Util::Text>(kUIFont, 30, text, outlineColor);
        auto shadowObject = std::make_shared<FloatingTextObject>(
            shadowDrawable,
            position + offset,
            glm::vec2{0.0f, 48.0f},
            outlineColor,
            98.0f,
            0.85f);
        AddDebugEntity(shadowObject, 0.85f);
    }

    auto frontDrawable = std::make_shared<Util::Text>(kUIFont, 30, text, frontColor);
    auto frontObject = std::make_shared<FloatingTextObject>(
        frontDrawable,
        position,
        glm::vec2{0.0f, 52.0f},
        frontColor,
        99.0f,
        0.85f);
    AddDebugEntity(frontObject, 0.85f);
}

void GameScene::SpawnFloatingScore(const glm::vec2 &position,
                                   const int points,
                                   const Util::Color &frontColor)
{
    if (points <= 0)
    {
        return;
    }

    SpawnOutlinedFloatingScore(position, FormatScore(points), frontColor);
}

void GameScene::FinalizeScoreForCharacter(const std::shared_ptr<Character> &character, const glm::vec2 &atPosition)
{
    if (!character)
    {
        return;
    }

    int awarded = 0;
    if (character->GetEntityKind() == Character::EntityKind::Pig)
    {
        awarded = m_ScoringSystem.AwardPigDestroyed();
        if (m_RemainingPigCount > 0)
        {
            --m_RemainingPigCount;
        }
        if (m_RemainingPigCount == 0)
        {
            m_LevelCleared = true;
        }
    }
    else if (character->IsSpecialItem())
    {
        awarded = m_ScoringSystem.AwardSpecialItemDestroyed();
    }
    else if (character->GetEntityKind() == Character::EntityKind::Environment)
    {
        awarded = m_ScoringSystem.AwardBlockDestroyed(character->GetMaterialType());
    }

    if (awarded > 0)
    {
        SpawnFloatingScore(atPosition, awarded, Util::Color::FromRGB(255, 255, 255));
    }

    UpdateScoreHud();
}

void GameScene::OnCharacterDeath(const std::shared_ptr<Character> &character)
{
    FinalizeScoreForCharacter(character, character ? character->GetPosition() : glm::vec2{0.0f, 0.0f});
}

void GameScene::TogglePauseMenu()
{
    if (m_IsLevelClearScreenVisible || m_IsLevelFailedScreenVisible)
    {
        return;
    }

    SetPauseMenuVisible(!m_IsPauseMenuVisible);
}

void GameScene::ToggleMusicMute()
{
    m_IsMusicMuted = !m_IsMusicMuted;

    if (m_IsMusicMuted)
    {
        Mix_PauseMusic();
    }
    else
    {
        Mix_ResumeMusic();
    }

    if (m_PauseMenu040Overlay)
    {
        m_PauseMenu040Overlay->SetVisible(m_IsPauseMenuVisible && m_IsMusicMuted);
    }
}

void GameScene::SetPauseMenuButtonsInputEnabled(const bool enabled)
{
    if (m_PauseMenu069)
    {
        m_PauseMenu069->SetInputEnabled(enabled);
    }
    if (m_PauseMenu082)
    {
        m_PauseMenu082->SetInputEnabled(enabled);
    }
    if (m_PauseMenu073)
    {
        m_PauseMenu073->SetInputEnabled(enabled);
    }
    if (m_PauseMenu005)
    {
        m_PauseMenu005->SetInputEnabled(enabled);
    }
    if (m_PauseMenu063)
    {
        m_PauseMenu063->SetInputEnabled(enabled);
    }
}
