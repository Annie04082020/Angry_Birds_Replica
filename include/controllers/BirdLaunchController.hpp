#ifndef BIRD_LAUNCH_CONTROLLER_HPP
#define BIRD_LAUNCH_CONTROLLER_HPP

#include <memory>
#include <vector>
#include <functional>

#include "Character.hpp"

class BirdLaunchController
{
public:
    bool LoadLevelObjects(const std::vector<std::shared_ptr<Character>> &objects);
    bool Update();
    void SetWorldFloorY(float y) { m_WorldFloorY = y; }
    void SetOnSpawnCharacter(std::function<void(std::shared_ptr<Character>)> callback) {
        m_OnSpawnCharacter = callback;
    }
    void SetPhysicsScale(float scale) { m_PhysicsScale = scale; }

    [[nodiscard]] bool IsHoldingBird() const { return m_IsHoldingBird; }
    [[nodiscard]] int GetRemainingBirdCountForBonus() const;
    [[nodiscard]] bool HasAnyBirdBeenLaunched() const { return m_HasAnyBirdBeenLaunched; }
    [[nodiscard]] bool HasBirdInFlight() const { return m_HasLaunchedBird; }
    [[nodiscard]] bool IsOutOfBirds() const;

private:
    bool HandleBirdLaunchPhysics();
    glm::vec2 GetMouseWorldPosition() const;
    void ActivateBirdByIndex(size_t index);

    std::vector<std::shared_ptr<Character>> m_BirdQueue;
    std::shared_ptr<Character> m_ActiveBird = nullptr;
    glm::vec2 m_BirdAnchorPosition{0.0f, 0.0f};
    glm::vec2 m_BirdVelocity{0.0f, 0.0f};
    size_t m_CurrentBirdIndex = 0;
    bool m_IsHoldingBird = false;
    bool m_HasLaunchedBird = false;
    bool m_HasAnyBirdBeenLaunched = false;
    float m_WorldFloorY = -294.0f;
    std::vector<std::shared_ptr<Character>> m_ActiveBirdsInFlight;
    std::function<void(std::shared_ptr<Character>)> m_OnSpawnCharacter = nullptr;
    bool m_HasSplit = false;
    float m_PhysicsScale = 1.0f;
    // stop detection now uses velocity/angle thresholds
};

#endif // BIRD_LAUNCH_CONTROLLER_HPP
