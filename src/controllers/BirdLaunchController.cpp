#include "BirdLaunchController.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <glm/glm.hpp>

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
        for (const auto &bird : m_BirdQueue)
        {
            if (!bird)
            {
                continue;
            }
            bird->SetVelocity({0.0f, 0.0f});
            bird->SetAngularVelocity(0.0f);
            bird->SetStatic(true);
        }
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
    constexpr float launchPower = 9.0f;
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
                m_ActiveBird->SetVelocity({0.0f, 0.0f});
                m_BirdVelocity = {0.0f, 0.0f};
                return true;
            }
        }
        else if (m_IsHoldingBird)
        {
            const glm::vec2 pullVector = m_ActiveBird->GetPosition() - m_BirdAnchorPosition;
            m_BirdVelocity = -pullVector * launchPower;
            m_ActiveBird->SetStatic(false);
            m_ActiveBird->SetVelocity(m_BirdVelocity);
            m_IsHoldingBird = false;
            m_HasLaunchedBird = true;
            return true;
        }

        return m_IsHoldingBird;
    }

    glm::vec2 velocity = m_ActiveBird->GetVelocity();
    // gravity applied centrally in Scene::Update
    m_ActiveBird->SetVelocity(velocity);
    m_BirdVelocity = velocity;

    glm::vec2 nextPos = m_ActiveBird->GetPosition() + velocity * dt;

    if (nextPos.y < floorY)
    {
        nextPos.y = floorY;
        m_ActiveBird->SetPosition(nextPos);
        m_ActiveBird->SetVelocity({0.0f, 0.0f});
        m_ActiveBird->SetAngularVelocity(0.0f);
        m_ActiveBird->SetSleeping(true);
        m_BirdVelocity = {0.0f, 0.0f};
        m_HasLaunchedBird = false;

        if (m_CurrentBirdIndex + 1 < m_BirdQueue.size())
        {
            ActivateBirdByIndex(m_CurrentBirdIndex + 1);
        }
        return true;
    }

    // If the bird has effectively come to rest (on an object or ground),
    // advance to next bird. Use static flag or a small velocity + angular
    // velocity threshold to detect stopping.
    const float stopSpeedThreshold = 10.0f;  // px/sec
    const float stopAngularThreshold = 5.0f; // rad/sec

    if (m_ActiveBird->IsStatic() || m_ActiveBird->IsSleeping() ||
        (glm::length(m_BirdVelocity) < stopSpeedThreshold && std::fabs(m_ActiveBird->GetAngularVelocity()) < stopAngularThreshold))
    {
        m_ActiveBird->SetSleeping(true);
        m_ActiveBird->SetVelocity({0.0f, 0.0f});
        m_ActiveBird->SetAngularVelocity(0.0f);
        m_HasLaunchedBird = false;
        m_BirdVelocity = {0.0f, 0.0f};
        if (m_CurrentBirdIndex + 1 < m_BirdQueue.size())
        {
            ActivateBirdByIndex(m_CurrentBirdIndex + 1);
        }
    }

    // Position integration is handled by Scene::Update -> Character::IntegratePhysics.
    // Keep controller responsible for launch state and gravity only.
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
    m_ActiveBird->SetVelocity({0.0f, 0.0f});
    m_ActiveBird->SetAngularVelocity(0.0f);
    m_ActiveBird->SetStatic(true);
    m_BirdVelocity = {0.0f, 0.0f};
    m_IsHoldingBird = false;
    m_HasLaunchedBird = false;
    m_ActiveBird->SetZIndex(0.0f);
}