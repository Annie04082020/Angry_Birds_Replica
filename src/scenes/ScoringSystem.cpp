#include "ScoringSystem.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include "JsonParseUtils.hpp"
#include <spdlog/spdlog.h>

namespace
{
    // Legacy helper functions - no longer used but keeping for reference
    // int DamageBudgetForMaterial(const Character::MaterialType materialType)
    // {
    //     switch (materialType)
    //     {
    //     case Character::MaterialType::Glass:
    //     case Character::MaterialType::Ice:
    //         return 500;
    //     case Character::MaterialType::Wood:
    //         return 800;
    //     case Character::MaterialType::Stone:
    //         return 1200;
    //     default:
    //         return 0;
    //     }
    // }

    // int DestroyedPointsForMaterial(const Character::MaterialType materialType)
    // {
    //     switch (materialType)
    //     {
    //     case Character::MaterialType::Glass:
    //     case Character::MaterialType::Ice:
    //         return 500;
    //     case Character::MaterialType::Wood:
    //         return 900;
    //     case Character::MaterialType::Stone:
    //         return 1300;
    //     default:
    //         return 0;
    //     }
    // }
}

ScoringSystem::ScoringSystem()
{
    // Initialize material configs with default values
    m_MaterialConfigs[static_cast<int>(Character::MaterialType::Glass)] = {500, 500, 1.0f};
    m_MaterialConfigs[static_cast<int>(Character::MaterialType::Ice)] = {500, 500, 1.0f};
    m_MaterialConfigs[static_cast<int>(Character::MaterialType::Wood)] = {800, 900, 1.0f};
    m_MaterialConfigs[static_cast<int>(Character::MaterialType::Stone)] = {1200, 1300, 1.0f};
}

bool ScoringSystem::LoadConfig(const std::string& configPath)
{
    std::ifstream file(configPath);
    if (!file.is_open())
    {
        spdlog::warn("Failed to open scoring config file: {}", configPath);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonContent = buffer.str();

    // Extract basic points
    std::string scoringContent;
    if (JsonParseUtils::ExtractObjectContent(jsonContent, "scoring", scoringContent))
    {
        std::string basicPointsContent;
        if (JsonParseUtils::ExtractObjectContent(scoringContent, "basic_points", basicPointsContent))
        {
            m_PigPoints = JsonParseUtils::ExtractInt(basicPointsContent, "pig_destroyed", 5000);
            m_LeftoverBirdPoints = JsonParseUtils::ExtractInt(basicPointsContent, "leftover_bird", 10000);
            m_SpecialItemPoints = JsonParseUtils::ExtractInt(basicPointsContent, "special_item", 3000);
        }

        // Extract material points
        std::string materialPointsContent;
        if (JsonParseUtils::ExtractObjectContent(scoringContent, "material_points", materialPointsContent))
        {
            // Glass
            std::string glassContent;
            if (JsonParseUtils::ExtractObjectContent(materialPointsContent, "glass", glassContent))
            {
                m_MaterialConfigs[static_cast<int>(Character::MaterialType::Glass)] = {
                    JsonParseUtils::ExtractInt(glassContent, "damage_budget", 500),
                    JsonParseUtils::ExtractInt(glassContent, "destroyed_points", 500),
                    JsonParseUtils::ExtractFloat(glassContent, "damage_multiplier", 1.0f)
                };
            }

            // Ice
            std::string iceContent;
            if (JsonParseUtils::ExtractObjectContent(materialPointsContent, "ice", iceContent))
            {
                m_MaterialConfigs[static_cast<int>(Character::MaterialType::Ice)] = {
                    JsonParseUtils::ExtractInt(iceContent, "damage_budget", 500),
                    JsonParseUtils::ExtractInt(iceContent, "destroyed_points", 500),
                    JsonParseUtils::ExtractFloat(iceContent, "damage_multiplier", 1.0f)
                };
            }

            // Wood
            std::string woodContent;
            if (JsonParseUtils::ExtractObjectContent(materialPointsContent, "wood", woodContent))
            {
                m_MaterialConfigs[static_cast<int>(Character::MaterialType::Wood)] = {
                    JsonParseUtils::ExtractInt(woodContent, "damage_budget", 800),
                    JsonParseUtils::ExtractInt(woodContent, "destroyed_points", 900),
                    JsonParseUtils::ExtractFloat(woodContent, "damage_multiplier", 1.0f)
                };
            }

            // Stone
            std::string stoneContent;
            if (JsonParseUtils::ExtractObjectContent(materialPointsContent, "stone", stoneContent))
            {
                m_MaterialConfigs[static_cast<int>(Character::MaterialType::Stone)] = {
                    JsonParseUtils::ExtractInt(stoneContent, "damage_budget", 1200),
                    JsonParseUtils::ExtractInt(stoneContent, "destroyed_points", 1300),
                    JsonParseUtils::ExtractFloat(stoneContent, "damage_multiplier", 1.0f)
                };
            }
        }
    }

    spdlog::info("Scoring config loaded successfully from: {}", configPath);
    return true;
}

void ScoringSystem::Reset()
{
    m_Score = 0;
    // Do not reset m_HighScore - it persists across levels
}

int ScoringSystem::AwardPigDestroyed()
{
    return AddScore(ApplyMultiplier(m_PigPoints));
}

int ScoringSystem::AwardLeftoverBirds(const int birdCount)
{
    return AddScore(ApplyMultiplier(std::max(0, birdCount) * m_LeftoverBirdPoints));
}

int ScoringSystem::AwardSpecialItemDestroyed()
{
    return AddScore(ApplyMultiplier(m_SpecialItemPoints));
}

int ScoringSystem::AwardBlockDamage(const Character::MaterialType materialType,
                                    const float damageRatio,
                                    const int remainingBudget)
{
    if (damageRatio <= 0.0f || remainingBudget <= 0)
    {
        return 0;
    }

    const int budget = GetDamageBudget(materialType);
    if (budget <= 0)
    {
        return 0;
    }

    const int requested = std::max(10, static_cast<int>(std::lround(static_cast<float>(budget) * std::clamp(damageRatio, 0.0f, 1.0f))));
    return AddScore(ApplyMultiplier(std::min(requested, remainingBudget)));
}

int ScoringSystem::AwardBlockDestroyed(const Character::MaterialType materialType)
{
    const int index = static_cast<int>(materialType);
    if (index >= 0 && index < 5)
    {
        return AddScore(ApplyMultiplier(m_MaterialConfigs[index].destroyedPoints));
    }
    return 0;
}

int ScoringSystem::GetDamageBudget(const Character::MaterialType materialType) const
{
    const int index = static_cast<int>(materialType);
    if (index >= 0 && index < 5)
    {
        return m_MaterialConfigs[index].damageBudget;
    }
    return 0;
}

int ScoringSystem::GetDestroyedPoints(const Character::MaterialType materialType) const
{
    const int index = static_cast<int>(materialType);
    if (index >= 0 && index < 5)
    {
        return m_MaterialConfigs[index].destroyedPoints;
    }
    return 0;
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

int ScoringSystem::ApplyMultiplier(int basePoints) const
{
    return static_cast<int>(basePoints * m_DifficultyMultiplier);
}
