#ifndef GAME_SCENE_HPP
#define GAME_SCENE_HPP

#include "DynamicBackground.hpp"
#include "LevelManager.hpp"
#include "Scene.hpp"

class GameScene : public Scene
{
public:
    explicit GameScene(std::shared_ptr<Util::GameObject> background)
        : Scene(background),
          m_DynamicBackground(std::dynamic_pointer_cast<DynamicBackground>(background))
    {
        // Game scene should not play menu BGM.
        SetBGM(nullptr);
        // Keep background static and let dragging control its motion.
        if (m_DynamicBackground)
        {
            m_DynamicBackground->SetSpeed(0.0f);
        }
    }

    bool LoadLevel(const std::string &levelPath);
    void Update() override;

private:
    void HandleBackgroundDrag();

    std::shared_ptr<LevelManager> m_LevelManager = std::make_shared<LevelManager>();
    std::shared_ptr<DynamicBackground> m_DynamicBackground = nullptr;
    bool m_IsHoldingBackground = false;
    bool m_IsDraggingBackground = false;
    float m_WorldOffsetX = 0.0f;
    float m_ZoomScrollAccumulator = 0.0f;
    glm::vec2 m_DragStartMousePos{0.0f, 0.0f};
    glm::vec2 m_LastMousePos{0.0f, 0.0f};
};

#endif // GAME_SCENE_HPP
