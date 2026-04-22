#include "scenes/CollisionUtils.hpp"
#include <cmath>
#include <cfloat>

namespace CollisionUtils
{
    float GetRestitution(Character::MaterialType mat)
    {
        switch (mat)
        {
        case Character::MaterialType::Flesh:
            return 0.2f;
        case Character::MaterialType::Wood:
            return 0.12f;
        case Character::MaterialType::Stone:
            return 0.06f;
        case Character::MaterialType::Glass:
        case Character::MaterialType::Ice:
            return 0.18f;
        default:
            return 0.1f;
        }
    }

    float GetFriction(Character::MaterialType mat)
    {
        switch (mat)
        {
        case Character::MaterialType::Flesh:
            return 0.6f;
        case Character::MaterialType::Wood:
            return 0.7f;
        case Character::MaterialType::Stone:
            return 0.9f;
        case Character::MaterialType::Glass:
        case Character::MaterialType::Ice:
            return 0.2f;
        default:
            return 0.5f;
        }
    }

    bool ComputeOBBMTV(const Character &A, const Character &B,
                       glm::vec2 &outNormal, float &outDepth,
                       glm::vec2 &outContactPoint)
    {
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

        glm::vec2 Ac, Au0, Au1, Bc, Bu0, Bu1;
        float Aex, Aey, Bex, Bey;
        buildOBB(A, Ac, Au0, Au1, Aex, Aey);
        buildOBB(B, Bc, Bu0, Bu1, Bex, Bey);

        auto testAxis = [&](const glm::vec2 &axis, float &outOverlap, glm::vec2 &outAxisDir) -> bool
        {
            float len = std::sqrt(axis.x * axis.x + axis.y * axis.y);
            if (len < 1e-6f)
                return true;
            glm::vec2 n = axis / len;
            float rA = Aex * std::fabs(glm::dot(n, Au0)) + Aey * std::fabs(glm::dot(n, Au1));
            float rB = Bex * std::fabs(glm::dot(n, Bu0)) + Bey * std::fabs(glm::dot(n, Bu1));
            float dist = glm::dot(Bc - Ac, n);
            float overlap = (rA + rB) - std::fabs(dist);
            outOverlap = overlap;
            outAxisDir = n;
            return overlap >= 0.0f;
        };

        // Test axes and find minimum penetration
        float minOverlap = FLT_MAX;
        glm::vec2 minAxis = glm::vec2(1.0f, 0.0f);
        glm::vec2 candidateAxis;
        float overlapVal = 0.0f;

        if (!testAxis(Au0, overlapVal, candidateAxis))
            return false;
        if (overlapVal < minOverlap)
        {
            minOverlap = overlapVal;
            minAxis = candidateAxis;
        }
        if (!testAxis(Au1, overlapVal, candidateAxis))
            return false;
        if (overlapVal < minOverlap)
        {
            minOverlap = overlapVal;
            minAxis = candidateAxis;
        }
        if (!testAxis(Bu0, overlapVal, candidateAxis))
            return false;
        if (overlapVal < minOverlap)
        {
            minOverlap = overlapVal;
            minAxis = candidateAxis;
        }
        if (!testAxis(Bu1, overlapVal, candidateAxis))
            return false;
        if (overlapVal < minOverlap)
        {
            minOverlap = overlapVal;
            minAxis = candidateAxis;
        }

        // Ensure normal points from A to B
        if (glm::dot(Bc - Ac, minAxis) < 0.0f)
            minAxis = -minAxis;

        outNormal = minAxis;
        outDepth = minOverlap;

        // Approximate contact point: project centers to midpoint and nudge by half penetration
        outContactPoint = (Ac + Bc) * 0.5f - outNormal * (outDepth * 0.5f);
        return true;
    }
}
