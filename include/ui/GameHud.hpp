#ifndef GAME_HUD_HPP
#define GAME_HUD_HPP

#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "ui/Button.hpp"

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <utility>

class GameHud
{
public:
    using AddElementFunction = std::function<void(const std::shared_ptr<Util::GameObject> &)>;

    void Build(const AddElementFunction &addElement, float hudScale);
    void UpdateLayout(const glm::vec2 &cameraPos, const glm::vec2 &viewportSize, float hudScale, float zoom);
    void SetScore(int score);
    void SetHighScore(int highScore);
    void SetControlsVisible(bool visible);
    [[nodiscard]] bool HasBuilt() const { return m_HasBuilt; }

    void SetOnTogglePause(std::function<void()> callback) { m_OnTogglePause = std::move(callback); }
    void SetOnRestart(std::function<void()> callback) { m_OnRestart = std::move(callback); }

private:
    std::array<std::shared_ptr<Util::GameObject>, 4> m_ScoreLabelOutline{};
    std::shared_ptr<Util::GameObject> m_ScoreLabel = nullptr;
    std::array<std::shared_ptr<Util::GameObject>, 4> m_ScoreValueOutline{};
    std::shared_ptr<Util::GameObject> m_ScoreValue = nullptr;
    std::array<std::shared_ptr<Util::Text>, 4> m_ScoreValueOutlineDrawables{};
    std::shared_ptr<Util::Text> m_ScoreValueDrawable = nullptr;

    std::array<std::shared_ptr<Util::GameObject>, 4> m_HighScoreLabelOutline{};
    std::shared_ptr<Util::GameObject> m_HighScoreLabel = nullptr;
    std::array<std::shared_ptr<Util::GameObject>, 4> m_HighScoreValueOutline{};
    std::shared_ptr<Util::GameObject> m_HighScoreValue = nullptr;
    std::array<std::shared_ptr<Util::Text>, 4> m_HighScoreValueOutlineDrawables{};
    std::shared_ptr<Util::Text> m_HighScoreValueDrawable = nullptr;

    std::shared_ptr<Button> m_PauseButton = nullptr;
    std::shared_ptr<Button> m_RestartButton = nullptr;
    std::function<void()> m_OnTogglePause = nullptr;
    std::function<void()> m_OnRestart = nullptr;
    bool m_HasBuilt = false;
};

#endif // GAME_HUD_HPP
