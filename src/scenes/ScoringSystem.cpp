#include "ScoringSystem.hpp"

#include <algorithm>
#include <cmath>

namespace
{
    int DamageBudgetForMaterial(const Character::MaterialType materialType)
    {
        switch (materialType)
        {
        case Character::MaterialType::Glass:
        case Character::MaterialType::Ice:
            return 500;
        case Character::MaterialType::Wood:
            return 800;
        case Character::MaterialType::Stone:
            return 1200;
        default:
            return 0;
        }
    }

    int DestroyedPointsForMaterial(const Character::MaterialType materialType)
    {
        switch (materialType)
        {
        case Character::MaterialType::Glass:
        case Character::MaterialType::Ice:
            return 500;
        case Character::MaterialType::Wood:
            return 900;
        case Character::MaterialType::Stone:
            return 1300;
        default:
            return 0;
        }
    }
}

void ScoringSystem::Reset()
{
    m_Score = 0;
    // Do not reset m_HighScore - it persists across levels
}

int ScoringSystem::AwardPigDestroyed()
{
    return AddScore(kPigPoints);
}

int ScoringSystem::AwardLeftoverBirds(const int birdCount)
{
    return AddScore(std::max(0, birdCount) * kLeftoverBirdPoints);
}

int ScoringSystem::AwardSpecialItemDestroyed()
{
    return AddScore(kSpecialItemPoints);
}

int ScoringSystem::AwardBlockDamage(const Character::MaterialType materialType,
                                    const float damageRatio,
                                    const int remainingBudget)
{
    if (damageRatio <= 0.0f || remainingBudget <= 0)
    {
        return 0;
    }

    const int budget = DamageBudgetForMaterial(materialType);
    if (budget <= 0)
    {
        return 0;
    }

    const int requested = std::max(10, static_cast<int>(std::lround(static_cast<float>(budget) * std::clamp(damageRatio, 0.0f, 1.0f))));
    return AddScore(std::min(requested, remainingBudget));
}

int ScoringSystem::AwardBlockDestroyed(const Character::MaterialType materialType)
{
    return AddScore(DestroyedPointsForMaterial(materialType));
}

int ScoringSystem::GetDamageBudget(const Character::MaterialType materialType)
{
    return DamageBudgetForMaterial(materialType);
}

int ScoringSystem::GetDestroyedPoints(const Character::MaterialType materialType)
{
    return DestroyedPointsForMaterial(materialType);
}

int ScoringSystem::AddScore(const int points)
{
    if (points <= 0)
    {
        return 0;
    }

    m_Score += points;
    
    // Update high score if current score exceeds it
    if (m_Score > m_HighScore)
    {
        m_HighScore = m_Score;
    }
    
    return points;
}
