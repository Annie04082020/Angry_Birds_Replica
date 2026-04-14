#include "SceneInputController.hpp"

#include <algorithm>
#include <utility>

#include "SDL.h"
#include "SDL_image.h"

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
        int refCount = 0;
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

    void FreeHandCursors()
    {
        auto &runtime = GetHandCursorRuntime();

        if (runtime.defaultCursor != nullptr)
        {
            SDL_FreeCursor(runtime.defaultCursor);
            runtime.defaultCursor = nullptr;
        }
        if (runtime.draggingCursor != nullptr)
        {
            SDL_FreeCursor(runtime.draggingCursor);
            runtime.draggingCursor = nullptr;
        }
        if (runtime.clickingCursor != nullptr)
        {
            SDL_FreeCursor(runtime.clickingCursor);
            runtime.clickingCursor = nullptr;
        }

        runtime.initialized = false;
        runtime.hasApplied = false;
        runtime.lastMode = HandCursorMode::Default;
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

SceneInputController::SceneInputController(std::shared_ptr<DynamicBackground> background,
                                           std::shared_ptr<LevelManager> levelManager)
    : m_LevelManager(std::move(levelManager)),
      m_DynamicBackground(std::move(background))
{
    ++GetHandCursorRuntime().refCount;
}

SceneInputController::~SceneInputController()
{
    auto &runtime = GetHandCursorRuntime();
    if (runtime.refCount > 0)
    {
        --runtime.refCount;
    }

    if (runtime.refCount == 0)
    {
        FreeHandCursors();
    }
}

bool SceneInputController::Update(bool isBirdHolding)
{
    HandleBackgroundDrag(isBirdHolding);

    const bool mousePressed = Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB);
    HandCursorMode cursorMode = HandCursorMode::Default;
    if (mousePressed && isBirdHolding)
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
    return m_IsDraggingBackground || isBirdHolding;
}

void SceneInputController::HandleBackgroundDrag(bool isBirdHolding)
{
    if (isBirdHolding)
    {
        m_IsHoldingBackground = false;
        m_IsDraggingBackground = false;
        return;
    }

    const glm::vec2 mousePos = Util::Input::GetCursorPosition();
    const bool mousePressed = Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB);
    constexpr float dragStartThreshold = 2.0f;

    const float currentZoom = Util::GetCameraZoom();
    const float viewportWidth = Util::GetViewportSize().x;
    const float maxPanOffsetX = std::max(0.0f, (1.0f - currentZoom) * viewportWidth);
    const float minWorldOffsetX = -maxPanOffsetX;
    const float maxWorldOffsetX = maxPanOffsetX;

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