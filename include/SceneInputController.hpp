#ifndef SCENE_INPUT_CONTROLLER_HPP
#define SCENE_INPUT_CONTROLLER_HPP

#include <memory>

#include "DynamicBackground.hpp"
#include "LevelManager.hpp"

class SceneInputController
{
public:
    SceneInputController(std::shared_ptr<DynamicBackground> background,
                         std::shared_ptr<LevelManager> levelManager);
    ~SceneInputController();

    bool Update(bool isBirdHolding);

private:
    void HandleBackgroundDrag(bool isBirdHolding);

    std::shared_ptr<LevelManager> m_LevelManager = nullptr;
    std::shared_ptr<DynamicBackground> m_DynamicBackground = nullptr;
    bool m_IsHoldingBackground = false;
    bool m_IsDraggingBackground = false;
    float m_WorldOffsetX = 0.0f;
    glm::vec2 m_DragStartMousePos{0.0f, 0.0f};
    glm::vec2 m_LastMousePos{0.0f, 0.0f};
};

#endif // SCENE_INPUT_CONTROLLER_HPP