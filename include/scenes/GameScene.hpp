#ifndef GAME_SCENE_HPP
#define GAME_SCENE_HPP

#include "DynamicBackground.hpp"
#include "LevelManager.hpp"
#include "BirdLaunchController.hpp"
#include "ScoringSystem.hpp"
#include "SceneInputController.hpp"
#include "Scene.hpp"
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "ui/Button.hpp"
#include <array>
#include <functional>
#include <deque>
#include <unordered_map>

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

private:
    void LoadLevelHighScore();
    void PersistLevelHighScore() const;
    void BuildLevelHud();
    void BuildBirdTrail();
    void UpdateBirdTrail();
    void ResetBirdTrail();
    void UpdateHudPositions();
    void UpdateScoreHud();
    void ResetScoreState();
    void RefreshRemainingPigCount();
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
    void SetPauseMenuVisible(bool visible);
    void TogglePauseMenu();
    void ToggleMusicMute();
    void SetPauseMenuButtonsInputEnabled(bool enabled);

    std::shared_ptr<LevelManager> m_LevelManager = std::make_shared<LevelManager>();
    std::shared_ptr<BirdLaunchController> m_BirdLaunchController = std::make_shared<BirdLaunchController>();
    std::shared_ptr<SceneInputController> m_SceneInputController = nullptr;
    std::shared_ptr<DynamicBackground> m_DynamicBackground = nullptr;
    std::vector<std::shared_ptr<Util::GameObject>> m_BirdTrailDots;
    std::unordered_map<const Character *, glm::vec2> m_BirdTrailLastEmitPositions;
    size_t m_BirdTrailNextDotIndex = 0;
    std::array<std::shared_ptr<Util::GameObject>, 4> m_ScoreLabelOutline{};
    std::shared_ptr<Util::GameObject> m_ScoreLabel = nullptr;
    std::array<std::shared_ptr<Util::GameObject>, 4> m_ScoreValueOutline{};
    std::shared_ptr<Util::GameObject> m_ScoreValue = nullptr;
    std::array<std::shared_ptr<Util::Text>, 4> m_ScoreValueOutlineDrawables{};
    std::array<std::shared_ptr<Util::GameObject>, 4> m_HighScoreLabelOutline{};
    std::shared_ptr<Util::Text> m_ScoreValueDrawable = nullptr;
    std::shared_ptr<Util::GameObject> m_HighScoreLabel = nullptr;
    std::array<std::shared_ptr<Util::GameObject>, 4> m_HighScoreValueOutline{};
    std::shared_ptr<Util::GameObject> m_HighScoreValue = nullptr;
    std::array<std::shared_ptr<Util::Text>, 4> m_HighScoreValueOutlineDrawables{};
    std::shared_ptr<Util::Text> m_HighScoreValueDrawable = nullptr;
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
    
    // Level Clear Screen UI
    std::shared_ptr<Util::GameObject> m_LevelClearBackdrop = nullptr;
    std::shared_ptr<Util::GameObject> m_LevelClearTitle = nullptr;
    std::array<std::shared_ptr<Util::GameObject>, 3> m_LevelClearStars{};
    std::array<std::shared_ptr<Util::GameObject>, 3> m_LevelClearEarnedStars{};
    std::array<std::shared_ptr<Util::GameObject>, 4> m_LevelClearScoreOutline{};
    std::shared_ptr<Util::GameObject> m_LevelClearScore = nullptr;
    std::shared_ptr<Util::Text> m_LevelClearScoreDrawable = nullptr;
    std::array<std::shared_ptr<Util::Text>, 4> m_LevelClearScoreOutlineDrawables{};
    std::array<std::shared_ptr<Util::GameObject>, 4> m_LevelClearHighScoreOutline{};
    std::shared_ptr<Util::GameObject> m_LevelClearHighScore = nullptr;
    std::shared_ptr<Util::Text> m_LevelClearHighScoreDrawable = nullptr;
    std::array<std::shared_ptr<Util::Text>, 4> m_LevelClearHighScoreOutlineDrawables{};
    std::array<std::shared_ptr<Util::GameObject>, 3> m_LevelClearBestStars{};
    std::shared_ptr<Button> m_LevelClearMenuButton = nullptr;
    std::shared_ptr<Button> m_LevelClearRestartButton = nullptr;
    std::shared_ptr<Button> m_LevelClearNextButton = nullptr;
    bool m_IsLevelClearScreenVisible = false;
    
    std::shared_ptr<Util::GameObject> m_LevelFailedBackdrop = nullptr;
    std::shared_ptr<Util::GameObject> m_LevelFailedTitle = nullptr;
    std::shared_ptr<Util::GameObject> m_LevelFailedPig = nullptr;
    std::shared_ptr<Button> m_LevelFailedMenuButton = nullptr;
    std::shared_ptr<Button> m_LevelFailedRestartButton = nullptr;
    std::shared_ptr<Button> m_LevelFailedNextButton = nullptr;
    bool m_IsLevelFailedScreenVisible = false;

    std::function<void()> m_OnRestartLevel = nullptr;
    std::function<void()> m_OnOpenLevelSelect = nullptr;
    std::function<void()> m_OnNextLevel = nullptr;
    bool m_IsPauseMenuVisible = false;
    bool m_PauseMenuInputBlockedUntilRelease = false;
    bool m_IsMusicMuted = false;
    float m_ZoomScrollAccumulator = 0.0f;
    float m_DamageOutputTimer = 0.0f;
    bool m_ShowDamageHud = false;
    ScoringSystem m_ScoringSystem;
    int m_HudHighScore = 0;
    int m_RemainingPigCount = 0;
    bool m_LevelCleared = false;
    bool m_LevelFailed = false;
    bool m_LeftoverBirdsAwarded = false;
    int m_PendingLeftoverBirdAwards = 0;
    std::deque<glm::vec2> m_PendingLeftoverBirdAwardPositions;
    float m_LeftoverBirdAwardTimer = 0.0f;
    float m_LevelClearAnimationTime = 0.0f;
};

#endif // GAME_SCENE_HPP
