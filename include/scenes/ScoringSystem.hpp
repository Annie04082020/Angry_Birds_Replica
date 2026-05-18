#ifndef SCORING_SYSTEM_HPP
#define SCORING_SYSTEM_HPP

#include "Character.hpp"

class ScoringSystem
{
public:
    static constexpr int kPigPoints = 5000;
    static constexpr int kLeftoverBirdPoints = 10000;
    static constexpr int kSpecialItemPoints = 3000;

    void Reset();
    [[nodiscard]] int GetScore() const { return m_Score; }
    [[nodiscard]] int GetHighScore() const { return m_HighScore; }
    void SetHighScore(int highScore) { m_HighScore = highScore; }

    int AwardPigDestroyed();
    int AwardLeftoverBirds(int birdCount);
    int AwardSpecialItemDestroyed();
    int AwardBlockDamage(Character::MaterialType materialType, float damageRatio, int remainingBudget);
    int AwardBlockDestroyed(Character::MaterialType materialType);

    [[nodiscard]] static int GetDamageBudget(Character::MaterialType materialType);
    [[nodiscard]] static int GetDestroyedPoints(Character::MaterialType materialType);

private:
    int AddScore(int points);

    int m_Score = 0;
    int m_HighScore = 0;
};

#endif // SCORING_SYSTEM_HPP
