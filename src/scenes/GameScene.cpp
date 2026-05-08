#include "GameScene.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "config.hpp"

#include "Resource.hpp"
#include "Util/DebugBox.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Text.hpp"
#include "Util/TransformUtils.hpp"
#include <SDL_mixer.h>

namespace
{
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
}

namespace
{
    constexpr float kGrassTopRatio = 404.0f / 563.0f;
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

    BuildLevelHud();
    UpdateHudPositions();

    m_SceneInputController = std::make_shared<SceneInputController>(m_DynamicBackground, m_LevelManager);

    return true;
}

void GameScene::Update()
{
    const bool isBirdHolding = m_BirdLaunchController && m_BirdLaunchController->IsHoldingBird();

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

    if (m_BirdLaunchController)
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
}

void GameScene::BuildLevelHud()
{
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

    SetPauseMenuVisible(false);
}

void GameScene::UpdateHudPositions()
{
    if (!m_LeftTopButton093 && !m_LeftTopButton031)
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

void GameScene::TogglePauseMenu()
{
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
