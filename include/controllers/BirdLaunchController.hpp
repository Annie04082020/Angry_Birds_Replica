#ifndef BIRD_LAUNCH_CONTROLLER_HPP
#define BIRD_LAUNCH_CONTROLLER_HPP

#include <memory>
#include <vector>

#include "Character.hpp"

class BirdLaunchController
{
public:
    bool LoadLevelObjects(const std::vector<std::shared_ptr<Character>> &objects);
    bool Update();

    [[nodiscard]] bool IsHoldingBird() const { return m_IsHoldingBird; }

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
};

#endif // BIRD_LAUNCH_CONTROLLER_HPP