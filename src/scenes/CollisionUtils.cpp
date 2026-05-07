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
            return 0.7f;
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

    bool ComputeOBBMTV(const Character &A, const Character &B,
                       glm::vec2 &outNormal, float &outDepth,
                       glm::vec2 &outContactPoint)
    {
        const auto polyA = BuildWorldCollider(A);
        const auto polyB = BuildWorldCollider(B);
        const glm::vec2 centerA = PolygonCentroid(polyA);
        const glm::vec2 centerB = PolygonCentroid(polyB);
        return ComputeConvexMTV(polyA, polyB, centerA, centerB, outNormal, outDepth, outContactPoint);
    }
}
