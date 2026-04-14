#ifndef GAME_SCENE_HPP
#define GAME_SCENE_HPP

#include "DynamicBackground.hpp"
#include "LevelManager.hpp"
#include "BirdLaunchController.hpp"
#include "SceneInputController.hpp"
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
        m_SceneInputController = std::make_shared<SceneInputController>(m_DynamicBackground, m_LevelManager);
    }

    bool LoadLevel(const std::string &levelPath);
    void Update() override;

private:
    std::shared_ptr<LevelManager> m_LevelManager = std::make_shared<LevelManager>();
    std::shared_ptr<BirdLaunchController> m_BirdLaunchController = std::make_shared<BirdLaunchController>();
    std::shared_ptr<SceneInputController> m_SceneInputController = nullptr;
    std::shared_ptr<DynamicBackground> m_DynamicBackground = nullptr;
    float m_ZoomScrollAccumulator = 0.0f;
};

#endif // GAME_SCENE_HPP
