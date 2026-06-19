#ifndef GAME_SCENE_HPP
#define GAME_SCENE_HPP

#include "DynamicBackground.hpp"
#include "LevelManager.hpp"
#include "BirdLaunchController.hpp"
#include "ScoringSystem.hpp"
#include "SceneInputController.hpp"
#include "Scene.hpp"
#include "effects/BirdTrail.hpp"
#include "effects/FloatingScoreManager.hpp"
#include "ui/GameHud.hpp"
#include "ui/LevelResultPanel.hpp"
#include "ui/PauseMenu.hpp"
#include <functional>
#include <deque>

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
    void SetOnNextLevelCallback(std::function<void()> callback)
    {
        m_OnNextLevel = std::move(callback);
    }
    void SetOnLevelClearedCallback(std::function<void(int)> callback)
    {
        m_OnLevelCleared = std::move(callback);
    }

private:
    void LoadLevelHighScore();
    void PersistLevelHighScore() const;
    void BuildLevelHud();
    void UpdateHudPositions();
    void UpdateScoreHud();
    void ResetScoreState();
    void RefreshRemainingPigCount();
    bool IsClearObjectiveComplete() const;
    void UpdateClearObjectiveState();
    void UpdateWinState();
    void UpdateFailState();
    void SpawnFloatingScore(const glm::vec2 &position, int points, const Util::Color &frontColor);
    void SpawnOutlinedFloatingScore(const glm::vec2 &position,
                                    const std::string &text,
                                    const Util::Color &frontColor,
                                    int fontSize,
                                    float lifeTime,
                                    const glm::vec2 &velocity);
    void FinalizeScoreForCharacter(const std::shared_ptr<Character> &character, const glm::vec2 &atPosition);
    void OnCharacterDeath(const std::shared_ptr<Character> &character) override;
    void OnCharacterHitFloor(const std::shared_ptr<Character> &character) override;
    void SetPauseMenuVisible(bool visible);
    void TogglePauseMenu();
    void ToggleMusicMute();

    std::shared_ptr<LevelManager> m_LevelManager = std::make_shared<LevelManager>();
    std::shared_ptr<BirdLaunchController> m_BirdLaunchController = std::make_shared<BirdLaunchController>();
    std::shared_ptr<SceneInputController> m_SceneInputController = nullptr;
    std::shared_ptr<DynamicBackground> m_DynamicBackground = nullptr;
    GameHud m_GameHud;
    LevelResultPanel m_LevelResultPanel;
    BirdTrail m_BirdTrail;
    FloatingScoreManager m_FloatingScoreManager;
    std::shared_ptr<PauseMenu> m_PauseMenu = nullptr;
    bool m_IsLevelClearScreenVisible = false;
    bool m_IsLevelFailedScreenVisible = false;

    std::function<void()> m_OnRestartLevel = nullptr;
    std::function<void()> m_OnOpenLevelSelect = nullptr;
    std::function<void()> m_OnNextLevel = nullptr;
    std::function<void(int)> m_OnLevelCleared = nullptr;
    bool m_IsPauseMenuVisible = false;
    bool m_PauseMenuInputBlockedUntilRelease = false;
    bool m_IsMusicMuted = SoundEffect::IsMuted();
    float m_ZoomScrollAccumulator = 0.0f;
    float m_DamageOutputTimer = 0.0f;
    bool m_ShowDamageHud = false;
    ScoringSystem m_ScoringSystem;
    int m_HudHighScore = 0;
    int m_RemainingPigCount = 0;
    bool m_LevelThreeRequiresSmileLanding = false;
    bool m_LevelThreeSmileLanded = false;
    bool m_LevelCleared = false;
    bool m_LevelFailed = false;
    bool m_LeftoverBirdsAwarded = false;
    int m_PendingLeftoverBirdAwards = 0;
    std::deque<glm::vec2> m_PendingLeftoverBirdAwardPositions;
    float m_LeftoverBirdAwardTimer = 0.0f;
    float m_LevelClearAnimationTime = 0.0f;
    
    // Intro Animation State
    bool m_IsIntroAnimating = false;
    float m_IntroTimer = 0.0f;
    float m_IntroCameraTargetX = 0.0f;
    float m_IntroDuration = 2.0f;
    float m_IntroWaitDuration = 1.0f;
};

#endif // GAME_SCENE_HPP
