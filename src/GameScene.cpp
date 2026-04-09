#include "GameScene.hpp"

#include <algorithm>
#include <cmath>

#include "SDL.h"
#include "SDL_image.h"
#include "config.hpp"

#include "Character.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/TransformUtils.hpp"

namespace
{
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

    if (!m_LevelManager || !m_LevelManager->LoadLevel(levelPath))
    {
        return false;
    }

    const auto &objects = m_LevelManager->GetGameObjects();
    for (const auto &obj : objects)
    {
        AddElements(obj);
    }

    if (!objects.empty())
    {
        auto controlled = std::dynamic_pointer_cast<Character>(objects.front());
        if (controlled)
        {
            SetControlledCharacter(controlled);
        }
    }

    return true;
}

void GameScene::Update()
{
    HandleBackgroundDrag();

    // Handle mouse wheel zoom with mouse position as pivot
    if (Util::Input::IfScroll())
    {
        const glm::vec2 mousePos = Util::Input::GetCursorPosition();
        const float oldZoom = Util::GetCameraZoom();
        const glm::vec2 oldCameraPos = Util::GetCameraPosition();

        // Calculate world position of mouse before zoom
        const glm::vec2 worldMousePos = mousePos / oldZoom + oldCameraPos;

        // Apply zoom
        const glm::vec2 scrollDist = Util::Input::GetScrollDistance();
        const float zoomDelta = scrollDist.y > 0 ? 1.05f : 0.95f;
        const float newZoom = oldZoom * zoomDelta;
        Util::SetCameraZoom(newZoom);

        // Adjust camera position so mouse position stays fixed
        const glm::vec2 newCameraPos = worldMousePos - mousePos / newZoom;
        Util::SetCameraPosition(newCameraPos);
    }

    const bool mousePressed = Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB);

    HandCursorMode cursorMode = HandCursorMode::Default;
    if (mousePressed && m_IsDraggingBackground)
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

void GameScene::HandleBackgroundDrag()
{
    const glm::vec2 mousePos = Util::Input::GetCursorPosition();
    const bool mousePressed = Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB);
    constexpr float dragStartThreshold = 2.0f;

    if (mousePressed)
    {
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
        const float minWorldOffsetX = -static_cast<float>(WINDOW_WIDTH);
        const float maxWorldOffsetX = static_cast<float>(WINDOW_WIDTH);
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
