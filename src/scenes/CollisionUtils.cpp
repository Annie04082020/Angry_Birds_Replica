#include "scenes/CollisionUtils.hpp"

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <vector>

namespace CollisionUtils
{
    namespace
    {
        glm::vec2 RotatePoint(const glm::vec2 &point, float radians)
        {
            const float c = std::cos(radians);
            const float s = std::sin(radians);
            return {point.x * c - point.y * s, point.x * s + point.y * c};
        }

        std::vector<glm::vec2> BuildLocalCollider(const Character &character)
        {
            const glm::vec2 half = character.GetSize() * 0.5f;
            switch (character.GetColliderShape())
            {
            case Character::ColliderShape::TriangleUp:
                return {{-half.x, -half.y}, {half.x, -half.y}, {0.0f, half.y}};
            case Character::ColliderShape::TriangleDown:
                return {{-half.x, half.y}, {half.x, half.y}, {0.0f, -half.y}};
            case Character::ColliderShape::TriangleLeft:
                return {{half.x, -half.y}, {half.x, half.y}, {-half.x, 0.0f}};
            case Character::ColliderShape::TriangleRight:
                return {{-half.x, -half.y}, {-half.x, half.y}, {half.x, 0.0f}};
            case Character::ColliderShape::Box:
            default:
                return {{-half.x, -half.y}, {half.x, -half.y}, {half.x, half.y}, {-half.x, half.y}};
            }
        }

        std::vector<glm::vec2> BuildWorldCollider(const Character &character)
        {
            const glm::vec2 center = character.GetPosition();
            const float rotation = character.m_Transform.rotation;
            const auto localPoints = BuildLocalCollider(character);

            std::vector<glm::vec2> worldPoints;
            worldPoints.reserve(localPoints.size());
            for (const auto &localPoint : localPoints)
            {
                worldPoints.push_back(center + RotatePoint(localPoint, rotation));
            }

            return worldPoints;
        }

        glm::vec2 PolygonCentroid(const std::vector<glm::vec2> &points)
        {
            if (points.empty())
            {
                return {0.0f, 0.0f};
            }

            glm::vec2 centroid{0.0f, 0.0f};
            for (const auto &point : points)
            {
                centroid += point;
            }
            return centroid / static_cast<float>(points.size());
        }

        bool ProjectPolygon(const std::vector<glm::vec2> &points,
                            const glm::vec2 &axis,
                            float &outMin,
                            float &outMax)
        {
            if (points.empty())
            {
                return false;
            }

            outMin = outMax = glm::dot(points.front(), axis);
            for (size_t i = 1; i < points.size(); ++i)
            {
                const float projection = glm::dot(points[i], axis);
                outMin = std::min(outMin, projection);
                outMax = std::max(outMax, projection);
            }
            return true;
        }

        bool TestAxis(const std::vector<glm::vec2> &polyA,
                      const std::vector<glm::vec2> &polyB,
                      const glm::vec2 &axis,
                      float &bestOverlap,
                      glm::vec2 &bestAxis)
        {
            float len = std::sqrt(axis.x * axis.x + axis.y * axis.y);
            if (len < 1e-6f)
            {
                return true;
            }

            const glm::vec2 n = axis / len;
            float minA = 0.0f;
            float maxA = 0.0f;
            float minB = 0.0f;
            float maxB = 0.0f;
            if (!ProjectPolygon(polyA, n, minA, maxA) || !ProjectPolygon(polyB, n, minB, maxB))
            {
                return false;
            }

            const float overlap = std::min(maxA, maxB) - std::max(minA, minB);
            if (overlap < 0.0f)
            {
                return false;
            }

            if (overlap < bestOverlap)
            {
                bestOverlap = overlap;
                bestAxis = n;
            }

            return true;
        }

        bool ComputeConvexMTV(const std::vector<glm::vec2> &polyA,
                              const std::vector<glm::vec2> &polyB,
                              const glm::vec2 &centerA,
                              const glm::vec2 &centerB,
                              glm::vec2 &outNormal,
                              float &outDepth,
                              glm::vec2 &outContactPoint)
        {
            if (polyA.size() < 3 || polyB.size() < 3)
            {
                return false;
            }

            float bestOverlap = FLT_MAX;
            glm::vec2 bestAxis{1.0f, 0.0f};

            auto testPolygonAxes = [&](const std::vector<glm::vec2> &poly) -> bool
            {
                for (size_t i = 0; i < poly.size(); ++i)
                {
                    const glm::vec2 edge = poly[(i + 1) % poly.size()] - poly[i];
                    const glm::vec2 axis{-edge.y, edge.x};
                    if (!TestAxis(polyA, polyB, axis, bestOverlap, bestAxis))
                    {
                        return false;
                    }
                }
                return true;
            };

            if (!testPolygonAxes(polyA) || !testPolygonAxes(polyB))
            {
                return false;
            }

            if (glm::dot(centerB - centerA, bestAxis) < 0.0f)
            {
                bestAxis = -bestAxis;
            }

            outNormal = bestAxis;
            outDepth = bestOverlap;
            outContactPoint = (centerA + centerB) * 0.5f - outNormal * (outDepth * 0.5f);
            return true;
        }
    } // namespace

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
        case Character::MaterialType::Earth:
            return 0.04f;
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
            return 1.2f; // High friction - wood is rough, especially the stage floor
        case Character::MaterialType::Stone:
            return 0.9f;
        case Character::MaterialType::Earth:
            return 0.95f;
        case Character::MaterialType::Glass:
        case Character::MaterialType::Ice:
            return 0.35f;
        default:
            return 0.5f;
        }
    }

    // Damage resistance factor: lower value = more easily damaged
    float GetDamageResistance(Character::MaterialType mat)
    {
        switch (mat)
        {
        case Character::MaterialType::Flesh:
            return 0.8f; // Living creatures take more damage
        case Character::MaterialType::Wood:
            return 1.0f; // Base damage resistance
        case Character::MaterialType::Stone:
            return 1.5f; // Stone is harder, needs more impact
        case Character::MaterialType::Glass:
            return 0.9f; // Glass is brittle
        case Character::MaterialType::Ice:
            return 0.7f; // Ice is fragile
        default:
            return 1.0f;
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
