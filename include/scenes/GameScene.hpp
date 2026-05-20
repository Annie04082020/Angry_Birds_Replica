#ifndef GAME_SCENE_HPP
#define GAME_SCENE_HPP

#include "DynamicBackground.hpp"
#include "LevelManager.hpp"
#include "BirdLaunchController.hpp"
#include "SceneInputController.hpp"
#include "Scene.hpp"
#include "Util/GameObject.hpp"
#include "ui/Button.hpp"
#include <functional>

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
    void SetOnRestartLevelCallback(std::function<void()> callback)
    {
        m_OnRestartLevel = std::move(callback);
    }
    void SetOnOpenLevelSelectCallback(std::function<void()> callback)
    {
        m_OnOpenLevelSelect = std::move(callback);
    }

private:
    void BuildLevelHud();
    void UpdateHudPositions();
    void SetPauseMenuVisible(bool visible);
    void TogglePauseMenu();
    void ToggleMusicMute();
    void SetPauseMenuButtonsInputEnabled(bool enabled);

    std::shared_ptr<LevelManager> m_LevelManager = std::make_shared<LevelManager>();
    std::shared_ptr<BirdLaunchController> m_BirdLaunchController = std::make_shared<BirdLaunchController>();
    std::shared_ptr<SceneInputController> m_SceneInputController = nullptr;
    std::shared_ptr<DynamicBackground> m_DynamicBackground = nullptr;
    std::shared_ptr<Button> m_LeftTopButton093 = nullptr;
    std::shared_ptr<Button> m_LeftTopButton031 = nullptr;
    std::shared_ptr<Button> m_PauseMenu069 = nullptr;
    std::shared_ptr<Button> m_PauseMenu082 = nullptr;
    std::shared_ptr<Button> m_PauseMenu073 = nullptr;
    std::shared_ptr<Button> m_PauseMenu005 = nullptr;
    std::shared_ptr<Button> m_PauseMenu063 = nullptr;
    std::shared_ptr<Util::GameObject> m_PauseMenu040Overlay = nullptr;
    std::shared_ptr<Util::GameObject> m_PauseMenuBackdrop = nullptr;
    std::shared_ptr<Util::GameObject> m_PauseMenuLevelTitle = nullptr;
    std::function<void()> m_OnRestartLevel = nullptr;
    std::function<void()> m_OnOpenLevelSelect = nullptr;
    bool m_IsPauseMenuVisible = false;
    bool m_PauseMenuInputBlockedUntilRelease = false;
    bool m_IsMusicMuted = false;
    float m_ZoomScrollAccumulator = 0.0f;
    float m_DamageOutputTimer = 0.0f;
    bool m_ShowDamageHud = false;
};

#endif // GAME_SCENE_HPP
