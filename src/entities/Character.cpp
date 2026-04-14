#include "Character.hpp"

#include <cmath>

#include "Util/Image.hpp"

Character::Character(const std::string &ImagePath)
{
    SetImage(ImagePath);

    ResetPosition();
}

void Character::SetImage(const std::string &ImagePath)
{
    m_ImagePath = ImagePath;

    m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
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
        deltaTimeSeconds <= 0.0f)
    {
        return;
    }

    m_Transform.translation += m_PhysicsState.velocity * deltaTimeSeconds;
    m_Transform.rotation +=
        m_PhysicsState.angularVelocity * deltaTimeSeconds;
}
