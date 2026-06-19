#ifndef LEVEL_RESULT_PANEL_HPP
#define LEVEL_RESULT_PANEL_HPP

#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "ui/Button.hpp"

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <utility>

class LevelResultPanel
{
public:
    using AddElementFunction = std::function<void(const std::shared_ptr<Util::GameObject> &)>;

    void Build(const AddElementFunction &addElement, float hudScale);
    void Reset();
    void ShowClear(const glm::vec2 &cameraPos,
                   const glm::vec2 &viewportSize,
                   float hudScale,
                   float zoom,
                   int score,
                   int stars,
                   int highScore,
                   int highScoreStars,
                   float animationTime);
    void ShowFailed(const glm::vec2 &cameraPos,
                    const glm::vec2 &viewportSize,
                    float hudScale,
                    float zoom);

    void SetOnOpenLevelSelect(std::function<void()> callback) { m_OnOpenLevelSelect = std::move(callback); }
    void SetOnRestart(std::function<void()> callback) { m_OnRestart = std::move(callback); }
    void SetOnNextLevel(std::function<void()> callback) { m_OnNextLevel = std::move(callback); }

private:
    std::shared_ptr<Util::GameObject> m_ClearBackdrop = nullptr;
    std::shared_ptr<Util::GameObject> m_ClearTitle = nullptr;
    std::array<std::shared_ptr<Util::GameObject>, 3> m_ClearStars{};
    std::array<std::shared_ptr<Util::GameObject>, 3> m_ClearEarnedStars{};
    std::array<std::shared_ptr<Util::GameObject>, 4> m_ClearScoreOutline{};
    std::shared_ptr<Util::GameObject> m_ClearScore = nullptr;
    std::shared_ptr<Util::Text> m_ClearScoreDrawable = nullptr;
    std::array<std::shared_ptr<Util::Text>, 4> m_ClearScoreOutlineDrawables{};
    std::array<std::shared_ptr<Util::GameObject>, 4> m_ClearHighScoreOutline{};
    std::shared_ptr<Util::GameObject> m_ClearHighScore = nullptr;
    std::shared_ptr<Util::Text> m_ClearHighScoreDrawable = nullptr;
    std::array<std::shared_ptr<Util::Text>, 4> m_ClearHighScoreOutlineDrawables{};
    std::array<std::shared_ptr<Util::GameObject>, 3> m_ClearBestStars{};
    std::shared_ptr<Button> m_ClearMenuButton = nullptr;
    std::shared_ptr<Button> m_ClearRestartButton = nullptr;
    std::shared_ptr<Button> m_ClearNextButton = nullptr;

    std::shared_ptr<Util::GameObject> m_FailedBackdrop = nullptr;
    std::shared_ptr<Util::GameObject> m_FailedTitle = nullptr;
    std::shared_ptr<Util::GameObject> m_FailedPig = nullptr;
    std::shared_ptr<Button> m_FailedMenuButton = nullptr;
    std::shared_ptr<Button> m_FailedRestartButton = nullptr;
    std::shared_ptr<Button> m_FailedNextButton = nullptr;

    std::function<void()> m_OnOpenLevelSelect = nullptr;
    std::function<void()> m_OnRestart = nullptr;
    std::function<void()> m_OnNextLevel = nullptr;
    bool m_HasBuilt = false;
};

#endif // LEVEL_RESULT_PANEL_HPP
