#include "Character.hpp"

#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>

#include "Util/Image.hpp"
#include "Util/Time.hpp"

Character::Character(const std::string &ImagePath)
{
    SetImage(ImagePath);

    ResetPosition();
}

void Character::SetImage(const std::string &ImagePath)
{
    if (m_ImagePath == ImagePath && m_Drawable)
    {
        return;
    }

    m_ImagePath = ImagePath;

    m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
}

void Character::Update()
{
    if (m_AnimationFramePaths.size() <= 1 || m_AnimationFrameDuration <= 0.0f)
    {
        return;
    }

    const float deltaSeconds = std::max(0.0f, Util::Time::GetDeltaTimeMs() / 1000.0f);
    if (deltaSeconds <= 0.0f)
    {
        return;
    }

    m_AnimationElapsed += deltaSeconds;
    while (m_AnimationElapsed >= m_AnimationFrameDuration)
    {
        m_AnimationElapsed -= m_AnimationFrameDuration;
        m_CurrentAnimationFrameIndex = (m_CurrentAnimationFrameIndex + 1) % m_AnimationFramePaths.size();
        SetImage(m_AnimationFramePaths[m_CurrentAnimationFrameIndex]);
    }
}

void Character::ConfigureLoopingAnimation(std::vector<std::string> framePaths, float frameDurationSeconds)
{
    m_AnimationFramePaths = std::move(framePaths);
    m_AnimationFrameDuration = std::max(0.0f, frameDurationSeconds);
    m_AnimationElapsed = 0.0f;
    m_CurrentAnimationFrameIndex = 0;

    if (!m_AnimationFramePaths.empty())
    {
        SetImage(m_AnimationFramePaths.front());
    }
}

void Character::ClearAnimation()
{
    m_AnimationFramePaths.clear();
    m_AnimationFrameDuration = 0.0f;
    m_AnimationElapsed = 0.0f;
    m_CurrentAnimationFrameIndex = 0;
}

void Character::SetPhysicsState(const PhysicsState &physicsState)
{
    m_PhysicsState = physicsState;

    if (m_PhysicsState.mass <= 0.0f)
    {
        m_PhysicsState.mass = 0.0001f;
    }

    if (m_PhysicsState.inertia <= 0.0f)
    {
        m_PhysicsState.inertia = 0.0001f;
    }
}

void Character::SetMass(float mass)
{
    m_PhysicsState.mass = (mass > 0.0f) ? mass : 0.0001f;
}

void Character::SetInertia(float inertia)
{
    m_PhysicsState.inertia = (inertia > 0.0f) ? inertia : 0.0001f;
}

void Character::IntegratePhysics(float deltaTimeSeconds)
{
    if (m_PhysicsState.isStatic || !std::isfinite(deltaTimeSeconds) ||
        deltaTimeSeconds <= 0.0f || m_PhysicsState.isSleeping)
    {
        return;
    }

    // Security clamp: Prevent explosive runaway velocities from intense impacts.
    // Box2D also clamps max translation/rotation per step.
    constexpr float kMaxVelocity = 4000.0f;
    constexpr float kMaxAngularVelocity = 25.0f;

    float speed = glm::length(m_PhysicsState.velocity);
    if (speed > kMaxVelocity)
    {
        m_PhysicsState.velocity = (m_PhysicsState.velocity / speed) * kMaxVelocity;
    }
    
    if (m_PhysicsState.angularVelocity > kMaxAngularVelocity)
        m_PhysicsState.angularVelocity = kMaxAngularVelocity;
    else if (m_PhysicsState.angularVelocity < -kMaxAngularVelocity)
        m_PhysicsState.angularVelocity = -kMaxAngularVelocity;

    m_Transform.translation += m_PhysicsState.velocity * deltaTimeSeconds;
    m_Transform.rotation += m_PhysicsState.angularVelocity * deltaTimeSeconds;

    // Apply linear damping to simulate air resistance and reduce explosive runaway velocities.
    constexpr float kLinearDamping = 0.25f; // per second
    m_PhysicsState.velocity *= std::exp(-kLinearDamping * deltaTimeSeconds);

    // Apply angular damping to reduce runaway spinning (exponential decay).
    // Reduced from 8.0f to 0.5f since the SI solver handles stability now.
    // 8.0f was actively killing all natural torque, preventing objects from tipping over!
    constexpr float kAngularDamping = 0.5f; // per second
    m_PhysicsState.angularVelocity *= std::exp(-kAngularDamping * deltaTimeSeconds);
}

// Broad-phase AABB then narrow-phase OBB (SAT) collision test
[[nodiscard]] bool Character::IfCollides(
    const std::shared_ptr<Character> &other) const
{
    // AABB broad-phase: quick reject
    auto aPos = GetPosition();
    auto bPos = other->GetPosition();
    auto aSize = GetSize();
    auto bSize = other->GetSize();
    if (std::fabs(aPos.x - bPos.x) > (aSize.x / 2.0f + bSize.x / 2.0f) ||
        std::fabs(aPos.y - bPos.y) > (aSize.y / 2.0f + bSize.y / 2.0f))
    {
        return false;
    }

    // Build OBB representation (center, two orthonormal axes, half-extents)
    auto buildOBB = [](const Character &c, glm::vec2 &center,
                       glm::vec2 &u0, glm::vec2 &u1, float &ex,
                       float &ey)
    {
        center = c.m_Transform.translation;
        float rot = c.m_Transform.rotation;
        u0 = glm::vec2(std::cos(rot), std::sin(rot));
        u1 = glm::vec2(-std::sin(rot), std::cos(rot));
        auto half = c.GetSize() * 0.5f;
        ex = half.x;
        ey = half.y;
    };

    glm::vec2 Acenter, A_u0, A_u1, Bcenter, B_u0, B_u1;
    float Aex, Aey, Bex, Bey;
    buildOBB(*this, Acenter, A_u0, A_u1, Aex, Aey);
    buildOBB(*other, Bcenter, B_u0, B_u1, Bex, Bey);

    auto testAxis = [&](const glm::vec2 &axis) -> bool
    {
        float len = std::sqrt(axis.x * axis.x + axis.y * axis.y);
        if (len < 1e-6f)
            return true; // degenerate axis, treat as overlap on this axis
        glm::vec2 n = axis / len;
        float rA = Aex * std::fabs(glm::dot(n, A_u0)) +
                   Aey * std::fabs(glm::dot(n, A_u1));
        float rB = Bex * std::fabs(glm::dot(n, B_u0)) +
                   Bey * std::fabs(glm::dot(n, B_u1));
        float dist = std::fabs(glm::dot(Bcenter - Acenter, n));
        return dist <= (rA + rB);
    };

    // SAT axes: both local axes (2 + 2)
    if (!testAxis(A_u0))
        return false;
    if (!testAxis(A_u1))
        return false;
    if (!testAxis(B_u0))
        return false;
    if (!testAxis(B_u1))
        return false;

    return true;
}
