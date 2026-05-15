#include "GameScene.hpp"

#include <algorithm>
#include <cmath>

#include "config.hpp"

#include "Resource.hpp"
#include "Util/Input.hpp"
#include "Util/TransformUtils.hpp"

namespace
{
    constexpr float kGrassTopRatio = 404.0f / 563.0f;
    constexpr float kGameHudButtonScale = 0.75f;
    constexpr float kGameHudLeftPadding = 52.0f;
    constexpr float kGameHudTopPadding = 52.0f;
    constexpr float kGameHudButtonSpacing = 92.0f;
}

bool GameScene::LoadLevel(const std::string &levelPath)
{
    m_ZoomScrollAccumulator = 0.0f;

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

    BuildLevelHud();
    UpdateHudPositions();

    m_SceneInputController = std::make_shared<SceneInputController>(m_DynamicBackground, m_LevelManager);

    return true;
}

void GameScene::Update()
{
    const bool isBirdHolding = m_BirdLaunchController && m_BirdLaunchController->IsHoldingBird();

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
