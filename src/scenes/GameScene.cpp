#include "GameScene.hpp"

#include <algorithm>
#include <cmath>

#include "config.hpp"

#include "Util/Input.hpp"
#include "Util/TransformUtils.hpp"

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

    // Determine world floor from wood stage bottom if present so objects land at stage edge
    float floorCandidate = this->GetWorldFloorY();
    bool foundStage = false;
    for (const auto &obj : objects)
    {
        if (!obj)
            continue;
        const std::string path = obj->GetImagePath();
        if (path.find("sprite_013") != std::string::npos || path.find("WOOD_stage") != std::string::npos)
        {
            const float bottom = obj->GetPosition().y - obj->GetSize().y * 0.5f;
            if (!foundStage || bottom > floorCandidate)
            {
                floorCandidate = bottom;
                foundStage = true;
            }
        }
    }
    if (foundStage)
    {
        this->SetWorldFloorY(floorCandidate);
    }

    if (m_BirdLaunchController)
    {
        m_BirdLaunchController->LoadLevelObjects(objects);
    }

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

    Scene::Update();
}
