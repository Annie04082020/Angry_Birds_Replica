#include "ScoringSystem.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
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
        // Extract star thresholds
        std::string starThresholdsContent;
        if (JsonParseUtils::ExtractArrayContent(scoringContent, "star_thresholds", starThresholdsContent))
        {
            m_StarThresholds.clear();
            std::stringstream ss(starThresholdsContent);
            std::string item;
            while (std::getline(ss, item, ','))
            {
                try
                {
                    m_StarThresholds.push_back(std::stoi(item));
                }
                catch (const std::exception&)
                {
                    // ignore invalid
                }
            }
        }
    }

    spdlog::info("Scoring config loaded successfully from: {}", configPath);
    return true;
}

bool ScoringSystem::LoadHighScoreFromFile(const std::string& filePath, const int levelNumber)
{
    m_HighScore = 0;

    if (levelNumber <= 0)
    {
        spdlog::warn("Invalid level number when loading high score: {}", levelNumber);
        return false;
    }

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        spdlog::info("High score file not found yet, starting fresh: {}", filePath);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    const std::string jsonContent = buffer.str();

    std::string highScoresContent;
    if (!JsonParseUtils::ExtractObjectContent(jsonContent, "high_scores", highScoresContent))
    {
        spdlog::warn("Missing high_scores object in high score file: {}", filePath);
        return false;
    }

    m_HighScore = std::max(0, JsonParseUtils::ExtractInt(highScoresContent, std::to_string(levelNumber), 0));
    return true;
}

bool ScoringSystem::SaveHighScoreToFile(const std::string& filePath, const int levelNumber) const
{
    if (levelNumber <= 0)
    {
        spdlog::warn("Invalid level number when saving high score: {}", levelNumber);
        return false;
    }

    std::map<int, int> highScores;
    std::ifstream inputFile(filePath);
    if (inputFile.is_open())
    {
        std::stringstream buffer;
        buffer << inputFile.rdbuf();
        const std::string jsonContent = buffer.str();

        std::string highScoresContent;
        if (JsonParseUtils::ExtractObjectContent(jsonContent, "high_scores", highScoresContent))
        {
            for (int currentLevel = 1; currentLevel <= 10; ++currentLevel)
            {
                highScores[currentLevel] = std::max(
                    0,
                    JsonParseUtils::ExtractInt(highScoresContent, std::to_string(currentLevel), 0));
            }
        }
    }

    const auto existingEntry = highScores.find(levelNumber);
    if (existingEntry != highScores.end() && existingEntry->second > m_HighScore)
    {
        highScores[levelNumber] = existingEntry->second;
    }
    else
    {
        highScores[levelNumber] = std::max(0, m_HighScore);
    }

    std::ofstream outputFile(filePath, std::ios::trunc);
    if (!outputFile.is_open())
    {
        spdlog::warn("Failed to save high score file: {}", filePath);
        return false;
    }

    outputFile << "{\n";
    outputFile << "  \"high_scores\": {\n";

    bool firstEntry = true;
    for (const auto& [storedLevel, storedScore] : highScores)
    {
        if (!firstEntry)
        {
            outputFile << ",\n";
        }

        outputFile << "    \"" << storedLevel << "\": " << storedScore;
        firstEntry = false;
    }

    outputFile << "\n  }\n";
    outputFile << "}\n";

    return true;
}

void ScoringSystem::Reset()
{
    m_Score = 0;
    // Do not reset m_HighScore - it persists across levels
}

void ScoringSystem::CommitCurrentScoreToHighScore()
{
    if (m_Score > m_HighScore)
    {
        m_HighScore = m_Score;
    }
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
    if (index >= 0 && index < 8)
    {
        return AddScore(ApplyMultiplier(m_MaterialConfigs[index].destroyedPoints));
    }
    return 0;
}

int ScoringSystem::GetDamageBudget(const Character::MaterialType materialType) const
{
    const int index = static_cast<int>(materialType);
    if (index >= 0 && index < 8)
    {
        return m_MaterialConfigs[index].damageBudget;
    }
    return 0;
}

int ScoringSystem::GetDestroyedPoints(const Character::MaterialType materialType) const
{
    const int index = static_cast<int>(materialType);
    if (index >= 0 && index < 8)
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
    return points;
}

int ScoringSystem::ApplyMultiplier(const int basePoints) const
{
    return static_cast<int>(static_cast<float>(basePoints) * m_DifficultyMultiplier);
}

int ScoringSystem::GetStarCount(int score) const
{
    int stars = 0;
    if (m_StarThresholds.empty())
    {
        // Fallback defaults if config failed to load or array was empty
        if (score >= 15000) stars = 1;
        if (score >= 30000) stars = 2;
        if (score >= 45000) stars = 3;
        return stars;
    }

    for (int threshold : m_StarThresholds)
    {
        if (score >= threshold)
        {
            stars++;
        }
        else
        {
            break;
        }
    }
    return std::min(stars, 3);
}
