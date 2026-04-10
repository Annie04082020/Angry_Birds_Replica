#include "GameScene.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>

#include "SDL.h"
#include "SDL_image.h"
#include "config.hpp"

#include "Character.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Time.hpp"
#include "Util/TransformUtils.hpp"

namespace
{
    std::string ToLowerCopy(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
                       [](unsigned char c)
                       { return static_cast<char>(std::tolower(c)); });
        return value;
    }

    enum class HandCursorMode
    {
        Default,
        Dragging,
        Clicking,
    };

    struct HandCursorRuntime
    {
        SDL_Cursor *defaultCursor = nullptr;  // sprite_002
        SDL_Cursor *draggingCursor = nullptr; // sprite_001
        SDL_Cursor *clickingCursor = nullptr; // sprite_000
        bool initialized = false;
        bool hasApplied = false;
        HandCursorMode lastMode = HandCursorMode::Default;
    };

    HandCursorRuntime &GetHandCursorRuntime()
    {
        static HandCursorRuntime runtime;
        return runtime;
    }

    SDL_Cursor *LoadHandCursor(const std::string &path, int hotX, int hotY)
    {
        SDL_Surface *surface = IMG_Load(path.c_str());
        if (!surface)
        {
            return nullptr;
        }

        SDL_Cursor *cursor = SDL_CreateColorCursor(surface, hotX, hotY);
        SDL_FreeSurface(surface);
        return cursor;
    }

    void EnsureHandCursorsInitialized()
    {
        auto &runtime = GetHandCursorRuntime();
        if (runtime.initialized)
        {
            return;
        }

        runtime.defaultCursor =
            LoadHandCursor(RESOURCE_DIR "/Image/hand/sprite_002.png", 8, 8);
        runtime.draggingCursor =
            LoadHandCursor(RESOURCE_DIR "/Image/hand/sprite_001.png", 8, 8);
        runtime.clickingCursor =
            LoadHandCursor(RESOURCE_DIR "/Image/hand/sprite_000.png", 8, 8);
        runtime.initialized = true;
    }

    void UpdateHandCursor(const HandCursorMode mode)
    {
        EnsureHandCursorsInitialized();

        auto &runtime = GetHandCursorRuntime();
        if (runtime.hasApplied && runtime.lastMode == mode)
        {
            return;
        }

        runtime.hasApplied = true;
        runtime.lastMode = mode;

        SDL_Cursor *target = runtime.defaultCursor;
        if (mode == HandCursorMode::Dragging)
        {
            target = runtime.draggingCursor;
        }
        else if (mode == HandCursorMode::Clicking)
        {
            target = runtime.clickingCursor;
        }

        if (!target)
        {
            return;
        }

        SDL_SetCursor(target);
        SDL_ShowCursor(SDL_ENABLE);
    }
} // namespace

bool GameScene::LoadLevel(const std::string &levelPath)
{
    m_WorldOffsetX = 0.0f;
    m_ZoomScrollAccumulator = 0.0f;
    m_IsHoldingBird = false;
    m_HasLaunchedBird = false;
    m_BirdVelocity = {0.0f, 0.0f};
    m_ActiveBird = nullptr;
    m_BirdQueue.clear();
    m_CurrentBirdIndex = 0;

    Util::SetCameraZoom(1.0f);
    Util::SetCameraPosition({0.0f, 0.0f});

    if (!m_LevelManager || !m_LevelManager->LoadLevel(levelPath))
    {
        return false;
    }

    const auto &objects = m_LevelManager->GetGameObjects();
    std::vector<glm::vec2> slingshotPositions;

    for (const auto &obj : objects)
    {
        AddElements(obj);

        auto character = std::dynamic_pointer_cast<Character>(obj);
        if (!character)
        {
            continue;
        }

        const std::string pathLower = ToLowerCopy(character->GetImagePath());
        // Match only actual bird asset directory, avoid false positives from
        // workspace path names (e.g. Angry_Birds_Replica).
        if (pathLower.find("/image/birds/") != std::string::npos)
        {
            m_BirdQueue.push_back(character);
            continue;
        }

        if (pathLower.find("slingshot") != std::string::npos ||
            pathLower.find("sprite_147") != std::string::npos ||
            pathLower.find("sprite_154") != std::string::npos)
        {
            slingshotPositions.push_back(character->GetPosition());
        }
    }

    if (!slingshotPositions.empty())
    {
        glm::vec2 center{0.0f, 0.0f};
        for (const auto &p : slingshotPositions)
        {
            center += p;
        }
        center /= static_cast<float>(slingshotPositions.size());
        // Place ready bird near the top of the slingshot.
        m_BirdAnchorPosition = center + glm::vec2(0.0f, 90.0f);
    }

    if (!m_BirdQueue.empty())
    {
        ActivateBirdByIndex(0);
    }

    return true;
}

void GameScene::Update()
{
    const bool isBirdInteractionActive = HandleBirdLaunchPhysics();

    if (!isBirdInteractionActive)
    {
        HandleBackgroundDrag();
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
            const float stepScale = zoomSteps > 0 ? (1.0f - zoomStep)
                                                  : (1.0f + zoomStep);
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

    const bool mousePressed = Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB);

    HandCursorMode cursorMode = HandCursorMode::Default;
    if (mousePressed && m_IsHoldingBird)
    {
        cursorMode = HandCursorMode::Dragging;
    }
    else if (mousePressed && m_IsDraggingBackground)
    {
        cursorMode = HandCursorMode::Dragging;
    }
    else if (mousePressed)
    {
        cursorMode = HandCursorMode::Clicking;
    }

    UpdateHandCursor(cursorMode);
    Scene::Update();
}

glm::vec2 GameScene::GetMouseWorldPosition() const
{
    const glm::vec2 mousePos = Util::Input::GetCursorPosition();
    const float zoom = Util::GetCameraZoom();
    const glm::vec2 cameraPos = Util::GetCameraPosition();
    return mousePos / zoom + cameraPos;
}

bool GameScene::HandleBirdLaunchPhysics()
{
    if (!m_ActiveBird)
    {
        return false;
    }

    const float dt = std::max(0.0f, Util::Time::GetDeltaTimeMs() / 1000.0f);
    const bool mousePressed = Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB);
    const glm::vec2 mouseWorldPos = GetMouseWorldPosition();

    constexpr float maxPullDistance = 140.0f;
    constexpr float launchPower = 7.0f;
    constexpr float gravity = 700.0f;
    constexpr float floorY = -320.0f;

    if (!m_HasLaunchedBird)
    {
        if (mousePressed)
        {
            if (!m_IsHoldingBird && m_ActiveBird->IsHovering(mouseWorldPos))
            {
                m_IsHoldingBird = true;
            }

            if (m_IsHoldingBird)
            {
                glm::vec2 pull = mouseWorldPos - m_BirdAnchorPosition;
                const float pullLength = glm::length(pull);
                if (pullLength > maxPullDistance && pullLength > 0.0f)
                {
                    pull = pull / pullLength * maxPullDistance;
                }

                m_ActiveBird->SetPosition(m_BirdAnchorPosition + pull);
                m_BirdVelocity = {0.0f, 0.0f};
                return true;
            }
        }
        else if (m_IsHoldingBird)
        {
            const glm::vec2 pullVector = m_ActiveBird->GetPosition() - m_BirdAnchorPosition;
            m_BirdVelocity = -pullVector * launchPower;
            m_IsHoldingBird = false;
            m_HasLaunchedBird = true;
            return true;
        }

        return m_IsHoldingBird;
    }

    m_BirdVelocity.y -= gravity * dt;
    glm::vec2 nextPos = m_ActiveBird->GetPosition() + m_BirdVelocity * dt;

    if (nextPos.y < floorY)
    {
        nextPos.y = floorY;
        m_ActiveBird->SetPosition(nextPos);
        m_BirdVelocity = {0.0f, 0.0f};
        m_HasLaunchedBird = false;

        if (m_CurrentBirdIndex + 1 < m_BirdQueue.size())
        {
            ActivateBirdByIndex(m_CurrentBirdIndex + 1);
        }
        return true;
    }

    m_ActiveBird->SetPosition(nextPos);
    return true;
}

void GameScene::ActivateBirdByIndex(size_t index)
{
    if (index >= m_BirdQueue.size())
    {
        return;
    }

    m_CurrentBirdIndex = index;
    m_ActiveBird = m_BirdQueue[index];
    if (!m_ActiveBird)
    {
        return;
    }

    m_ActiveBird->SetPosition(m_BirdAnchorPosition);
    m_BirdVelocity = {0.0f, 0.0f};
    m_IsHoldingBird = false;
    m_HasLaunchedBird = false;
    SetControlledCharacter(m_ActiveBird);
}

void GameScene::HandleBackgroundDrag()
{
    const glm::vec2 mousePos = Util::Input::GetCursorPosition();
    const bool mousePressed = Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB);
    constexpr float dragStartThreshold = 2.0f;

    const float currentZoom = Util::GetCameraZoom();
    const float viewportWidth = Util::GetViewportSize().x;
    const float maxPanOffsetX = std::max(0.0f, (1.0f - currentZoom) * viewportWidth);
    const float minWorldOffsetX = -maxPanOffsetX;
    const float maxWorldOffsetX = maxPanOffsetX;

    // If zoom range shrinks, clamp world offset and apply correction immediately.
    const float clampedOffsetX = std::clamp(m_WorldOffsetX, minWorldOffsetX, maxWorldOffsetX);
    const float correctionDeltaX = clampedOffsetX - m_WorldOffsetX;
    if (correctionDeltaX != 0.0f)
    {
        m_WorldOffsetX = clampedOffsetX;
        const glm::vec2 correctionDelta{correctionDeltaX, 0.0f};

        if (m_DynamicBackground)
        {
            m_DynamicBackground->Translate(correctionDelta);
        }

        if (m_LevelManager)
        {
            const auto &objects = m_LevelManager->GetGameObjects();
            for (const auto &obj : objects)
            {
                auto character = std::dynamic_pointer_cast<Character>(obj);
                if (!character)
                {
                    continue;
                }
                character->SetPosition(character->GetPosition() + correctionDelta);
            }
        }
    }

    if (mousePressed)
    {
        if (maxPanOffsetX <= 0.0f)
        {
            m_IsHoldingBackground = true;
            m_IsDraggingBackground = false;
            m_DragStartMousePos = mousePos;
            m_LastMousePos = mousePos;
            return;
        }

        if (!m_IsHoldingBackground)
        {
            m_IsHoldingBackground = true;
            m_IsDraggingBackground = false;
            m_DragStartMousePos = mousePos;
            m_LastMousePos = mousePos;
            return;
        }

        if (!m_IsDraggingBackground)
        {
            const glm::vec2 dragFromStart = mousePos - m_DragStartMousePos;
            if (std::abs(dragFromStart.x) >= dragStartThreshold)
            {
                m_IsDraggingBackground = true;
            }
        }

        if (!m_IsDraggingBackground)
        {
            m_LastMousePos = mousePos;
            return;
        }

        const float rawDeltaX = mousePos.x - m_LastMousePos.x;
        const float clampedWorldOffsetX =
            std::clamp(m_WorldOffsetX + rawDeltaX, minWorldOffsetX, maxWorldOffsetX);
        const float appliedDeltaX = clampedWorldOffsetX - m_WorldOffsetX;
        m_WorldOffsetX = clampedWorldOffsetX;

        const glm::vec2 delta{appliedDeltaX, 0.0f};

        // Move whole world as a camera-pan effect: background + all level objects.
        if (m_DynamicBackground && appliedDeltaX != 0.0f)
        {
            m_DynamicBackground->Translate(delta);
        }

        if (m_LevelManager && appliedDeltaX != 0.0f)
        {
            const auto &objects = m_LevelManager->GetGameObjects();
            for (const auto &obj : objects)
            {
                auto character = std::dynamic_pointer_cast<Character>(obj);
                if (!character)
                {
                    continue;
                }
                character->SetPosition(character->GetPosition() + delta);
            }
        }

        m_LastMousePos = mousePos;
    }
    else
    {
        m_IsHoldingBackground = false;
        m_IsDraggingBackground = false;
    }
}
