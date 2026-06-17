#ifndef BIRD_LAUNCH_CONTROLLER_HPP
#define BIRD_LAUNCH_CONTROLLER_HPP

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#include "Character.hpp"

class SoundEffect;

class BirdLaunchController
{
public:
    enum class IdleActionType
    {
        None,
        Hop,
        ForwardFlip,
        BackwardFlip
    };

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
    [[nodiscard]] int GetLaunchSequence() const { return m_LaunchSequence; }
    [[nodiscard]] std::vector<glm::vec2> GetRemainingBirdPositionsForBonus() const;
    [[nodiscard]] const std::vector<std::shared_ptr<Character>> &GetActiveBirdsInFlight() const
    {
        return m_ActiveBirdsInFlight;
    }
    [[nodiscard]] const glm::vec2 &GetBirdAnchorPosition() const
    {
        return m_BirdAnchorPosition;
    }

private:
    struct IdleAnimationState
    {
        glm::vec2 basePosition{0.0f, 0.0f};
        float baseRotation = 0.0f;
        float actionTimer = 0.0f;
        float actionCooldown = 0.0f;
        float actionDuration = 0.0f;
        IdleActionType actionType = IdleActionType::None;
        float vocalCooldown = 0.0f;
    };

    bool HandleBirdLaunchPhysics();
    glm::vec2 GetMouseWorldPosition() const;
    void ActivateBirdByIndex(size_t index);
    void UpdateQueuedBirdIdleAnimations(float deltaTimeSeconds);
    void ResetQueuedBirdIdleAnimation(const std::shared_ptr<Character> &bird, bool snapToBasePosition);
    void PlayRandomBirdVocal();
    void UpdatePigIdleVocals(float deltaTimeSeconds);
    void PlayRandomPigVocal();

    std::vector<std::shared_ptr<Character>> m_BirdQueue;
    std::vector<std::shared_ptr<Character>> m_IdlePigs;
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
    int m_LaunchSequence = 0;
    float m_ActiveBirdBaseRotation = 0.0f;
    std::unordered_map<const Character *, IdleAnimationState> m_QueuedBirdIdleStates;
    std::unordered_map<const Character *, float> m_PigVocalCooldowns;
    std::vector<std::shared_ptr<SoundEffect>> m_BirdIdleVocalSfx;
    std::vector<std::shared_ptr<SoundEffect>> m_PigIdleVocalSfx;
    float m_ActiveBirdVocalCooldown = 0.0f;
    // stop detection now uses velocity/angle thresholds
};

#endif // BIRD_LAUNCH_CONTROLLER_HPP
