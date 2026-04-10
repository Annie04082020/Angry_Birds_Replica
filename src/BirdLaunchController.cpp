#include "BirdLaunchController.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>

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
} // namespace

bool BirdLaunchController::LoadLevelObjects(const std::vector<std::shared_ptr<Character>> &objects)
{
    m_BirdQueue.clear();
    m_ActiveBird = nullptr;
    m_CurrentBirdIndex = 0;
    m_IsHoldingBird = false;
    m_HasLaunchedBird = false;
    m_BirdVelocity = {0.0f, 0.0f};

    std::vector<glm::vec2> slingshotPositions;

    for (const auto &obj : objects)
    {
        if (!obj)
        {
            continue;
        }

        const std::string pathLower = ToLowerCopy(obj->GetImagePath());
        if (pathLower.find("/image/birds/") != std::string::npos)
        {
            m_BirdQueue.push_back(obj);
            continue;
        }

        if (pathLower.find("slingshot") != std::string::npos ||
            pathLower.find("sprite_147") != std::string::npos ||
            pathLower.find("sprite_154") != std::string::npos)
        {
            slingshotPositions.push_back(obj->GetPosition());
        }
    }

    if (!slingshotPositions.empty())
    {
        glm::vec2 center{0.0f, 0.0f};
        for (const auto &position : slingshotPositions)
        {
            center += position;
        }
        center /= static_cast<float>(slingshotPositions.size());
        m_BirdAnchorPosition = center + glm::vec2(0.0f, 90.0f);
    }

    if (!m_BirdQueue.empty())
    {
        ActivateBirdByIndex(0);
    }

    return true;
}

bool BirdLaunchController::Update()
{
    return HandleBirdLaunchPhysics();
}

glm::vec2 BirdLaunchController::GetMouseWorldPosition() const
{
    const glm::vec2 mousePos = Util::Input::GetCursorPosition();
    const float zoom = Util::GetCameraZoom();
    const glm::vec2 cameraPos = Util::GetCameraPosition();
    return mousePos / zoom + cameraPos;
}

bool BirdLaunchController::HandleBirdLaunchPhysics()
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

void BirdLaunchController::ActivateBirdByIndex(size_t index)
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
    m_ActiveBird->SetZIndex(0.0f);
}