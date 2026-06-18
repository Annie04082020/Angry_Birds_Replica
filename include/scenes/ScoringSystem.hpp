#ifndef SCORING_SYSTEM_HPP
#define SCORING_SYSTEM_HPP

#include "Character.hpp"
#include <string>

class ScoringSystem
{
public:
    // Constructor and configuration loading
    ScoringSystem();
    bool LoadConfig(const std::string& configPath);
    bool LoadHighScoreFromFile(const std::string& filePath, int levelNumber);
    bool SaveHighScoreToFile(const std::string& filePath, int levelNumber) const;

    void Reset();
    [[nodiscard]] int GetScore() const { return m_Score; }
    [[nodiscard]] int GetHighScore() const { return m_HighScore; }
    void SetHighScore(int highScore) { m_HighScore = highScore; }
    void CommitCurrentScoreToHighScore();
    
    // Difficulty multiplier
    void SetDifficultyMultiplier(float multiplier) { m_DifficultyMultiplier = multiplier; }
    [[nodiscard]] float GetDifficultyMultiplier() const { return m_DifficultyMultiplier; }
    [[nodiscard]] int GetStarCount(int score) const;

    int AwardPigDestroyed();
    int AwardLeftoverBirds(int birdCount);
    int AwardSpecialItemDestroyed();
    int AwardBlockDamage(Character::MaterialType materialType, float damageRatio, int remainingBudget);
    int AwardBlockDestroyed(Character::MaterialType materialType);

    [[nodiscard]] int GetDamageBudget(Character::MaterialType materialType) const;
    [[nodiscard]] int GetDestroyedPoints(Character::MaterialType materialType) const;

private:
    int AddScore(int points);
    int ApplyMultiplier(int basePoints) const;

    int m_Score = 0;
    int m_HighScore = 0;
    float m_DifficultyMultiplier = 1.0f;
    std::vector<int> m_StarThresholds;

    // Points from config
    int m_PigPoints = 5000;
    int m_LeftoverBirdPoints = 10000;
    int m_SpecialItemPoints = 3000;

    // Material points from config
    struct MaterialConfig
    {
        int damageBudget = 0;
        int destroyedPoints = 0;
        float damageMultiplier = 1.0f;
    };

    MaterialConfig m_MaterialConfigs[8]; // For different material types
};

#endif // SCORING_SYSTEM_HPP
