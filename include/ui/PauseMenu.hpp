#ifndef PAUSE_MENU_HPP
#define PAUSE_MENU_HPP

#include "Util/GameObject.hpp"
#include "ui/Button.hpp"

#include <functional>
#include <memory>
#include <utility>

class PauseMenu
{
public:
    using AddElementFunction = std::function<void(const std::shared_ptr<Util::GameObject> &)>;

    void Build(const AddElementFunction &addElement, float hudScale);
    void UpdateLayout(const glm::vec2 &cameraPos, const glm::vec2 &viewportSize, float hudScale, float zoom);
    void SetVisible(bool visible, bool isMusicMuted, int levelNumber);
    void SetButtonsInputEnabled(bool enabled);
    void SetMusicMuted(bool isMusicMuted);

    void SetOnClose(std::function<void()> callback) { m_OnClose = std::move(callback); }
    void SetOnRestart(std::function<void()> callback) { m_OnRestart = std::move(callback); }
    void SetOnOpenLevelSelect(std::function<void()> callback) { m_OnOpenLevelSelect = std::move(callback); }
    void SetOnToggleMute(std::function<void()> callback) { m_OnToggleMute = std::move(callback); }

private:
    void RefreshLevelTitle(int levelNumber);

    std::shared_ptr<Button> m_CloseButton = nullptr;
    std::shared_ptr<Button> m_RestartButton = nullptr;
    std::shared_ptr<Button> m_MenuButton = nullptr;
    std::shared_ptr<Button> m_MuteButton = nullptr;
    std::shared_ptr<Button> m_SoundButton = nullptr;
    std::shared_ptr<Util::GameObject> m_MuteOverlay = nullptr;
    std::shared_ptr<Util::GameObject> m_Backdrop = nullptr;
    std::shared_ptr<Util::GameObject> m_LevelTitle = nullptr;

    std::function<void()> m_OnClose = nullptr;
    std::function<void()> m_OnRestart = nullptr;
    std::function<void()> m_OnOpenLevelSelect = nullptr;
    std::function<void()> m_OnToggleMute = nullptr;
    bool m_IsVisible = false;
    int m_CurrentLevelTitle = 0;
};

#endif // PAUSE_MENU_HPP
