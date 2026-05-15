#include "GameScene.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "config.hpp"

#include "Resource.hpp"
#include "Util/Input.hpp"
#include "Util/Text.hpp"
#include "Util/TransformUtils.hpp"

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

    constexpr float kGameHudButtonScale = 0.75f;
    constexpr float kGameHudLeftPadding = 52.0f;
    constexpr float kGameHudTopPadding = 52.0f;
    constexpr float kGameHudButtonSpacing = 92.0f;
}

bool GameScene::LoadLevel(const std::string &levelPath)
{
    m_ZoomScrollAccumulator = 0.0f;
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
    static float damageOutputTimer = 0.0f;
    if (m_ShowDamageHud && m_LevelManager)
    {
        damageOutputTimer += Util::Time::GetDeltaTimeMs() / 1000.0f;
        if (damageOutputTimer >= 2.0f) // Output every 2 seconds
        {
            damageOutputTimer = 0.0f;
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
    AddElements(m_LeftTopButton093);

    m_LeftTopButton039 = std::make_shared<Button>(Resource::Game_Button_039);
    m_LeftTopButton039->SetZIndex(95.0f);
    m_LeftTopButton039->SetScale({kGameHudButtonScale, kGameHudButtonScale});
    m_LeftTopButton039->SetVisible(true);
    AddElements(m_LeftTopButton039);
}

void GameScene::UpdateHudPositions()
{
    if (!m_LeftTopButton093 && !m_LeftTopButton039)
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

    if (m_LeftTopButton039)
    {
        m_LeftTopButton039->SetPosition(topLeftAnchor + glm::vec2{kGameHudButtonSpacing / zoom, 0.0f});
    }
}
