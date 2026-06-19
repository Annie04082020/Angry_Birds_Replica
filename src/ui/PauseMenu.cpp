#include "ui/PauseMenu.hpp"

#include "Resource.hpp"
#include "Util/DebugBox.hpp"
#include "Util/Image.hpp"

#include <string>

namespace
{
    constexpr float kBackdropWidth = 480.0f;
    constexpr float kBackdropHeightRatio = 1.15f;
    constexpr float kBackdropCenterOffsetX = 100.0f;
    constexpr float kBackdropCenterOffsetY = 10.0f;
    constexpr float kCloseOffsetX = 125.0f;
    constexpr float kCloseOffsetY = 95.0f;
    constexpr float kCloseScale = 1.2f;
    constexpr float kRestartOffsetX = 125.0f;
    constexpr float kRestartOffsetY = 250.0f;
    constexpr float kRestartScale = 1.25f;
    constexpr float kMenuOffsetX = 125.0f;
    constexpr float kMenuOffsetY = 400.0f;
    constexpr float kMenuScale = 1.25f;
    constexpr float kMuteOffsetX = 90.0f;
    constexpr float kMuteOffsetY = 900.0f;
    constexpr float kMuteScale = 1.2f;
    constexpr float kMuteOverlayOffsetX = 90.0f;
    constexpr float kMuteOverlayOffsetY = 895.0f;
    constexpr float kMuteOverlayScale = 1.2f;
    constexpr float kSoundOffsetX = 180.0f;
    constexpr float kSoundOffsetY = 900.0f;
    constexpr float kSoundScale = 1.2f;
    constexpr float kLevelTitleOffsetX = 280.0f;
    constexpr float kLevelTitleOffsetY = 450.0f;
    constexpr float kLevelTitleScale = 0.5f;

    std::string GetLevelTitleResource(const int levelNumber)
    {
        switch (levelNumber)
        {
        case 1:
            return Resource::Level_Title_1_1;
        case 2:
            return Resource::Level_Title_1_2;
        case 3:
            return Resource::Level_Title_1_3;
        case 4:
            return Resource::Level_Title_1_4;
        case 5:
            return Resource::Level_Title_1_5;
        case 6:
            return Resource::Level_Title_1_6;
        case 7:
            return Resource::Level_Title_1_7;
        case 8:
            return Resource::Level_Title_1_8;
        case 9:
            return Resource::Level_Title_1_9;
        case 10:
            return Resource::Level_Title_1_10;
        default:
            return {};
        }
    }

    void PositionButton(const std::shared_ptr<Button> &button,
                        const glm::vec2 &anchor,
                        const glm::vec2 &offset,
                        const float scale,
                        const float hudScale,
                        const float zoom)
    {
        if (!button)
        {
            return;
        }

        button->SetPosition(anchor + glm::vec2{(offset.x * hudScale) / zoom, -(offset.y * hudScale) / zoom});
        button->SetScale({scale * hudScale / zoom, scale * hudScale / zoom});
    }
}

void PauseMenu::Build(const AddElementFunction &addElement, const float hudScale)
{
    if (!addElement)
    {
        return;
    }

    m_Backdrop = std::make_shared<Util::GameObject>(
        std::make_shared<Util::DebugBox>(glm::vec4{0.0f, 0.0f, 0.0f, 0.58f}, 1.0f), 95.0f);
    m_Backdrop->SetVisible(false);
    addElement(m_Backdrop);

    m_CloseButton = std::make_shared<Button>(Resource::Game_Menu_Item_069);
    m_CloseButton->SetZIndex(96.0f);
    m_CloseButton->SetScale({kCloseScale * hudScale, kCloseScale * hudScale});
    m_CloseButton->SetVisible(false);
    m_CloseButton->SetSFX(Resource::SETTING_SFX);
    m_CloseButton->SetOnClickFunction([this]()
                                      {
                                          if (m_OnClose)
                                          {
                                              m_OnClose();
                                          }
                                      });
    addElement(m_CloseButton);

    m_RestartButton = std::make_shared<Button>(Resource::Game_Menu_Item_082);
    m_RestartButton->SetZIndex(96.0f);
    m_RestartButton->SetScale({kRestartScale * hudScale, kRestartScale * hudScale});
    m_RestartButton->SetVisible(false);
    m_RestartButton->SetSFX(Resource::SETTING_SFX);
    m_RestartButton->SetOnClickFunction([this]()
                                        {
                                            if (m_OnRestart)
                                            {
                                                m_OnRestart();
                                            }
                                        });
    addElement(m_RestartButton);

    m_MenuButton = std::make_shared<Button>(Resource::Game_Menu_Item_073);
    m_MenuButton->SetZIndex(96.0f);
    m_MenuButton->SetScale({kMenuScale * hudScale, kMenuScale * hudScale});
    m_MenuButton->SetVisible(false);
    m_MenuButton->SetSFX(Resource::SETTING_SFX);
    m_MenuButton->SetOnClickFunction([this]()
                                    {
                                        if (m_OnOpenLevelSelect)
                                        {
                                            m_OnOpenLevelSelect();
                                        }
                                    });
    addElement(m_MenuButton);

    m_MuteButton = std::make_shared<Button>(Resource::Game_Menu_Item_005);
    m_MuteButton->SetZIndex(96.0f);
    m_MuteButton->SetScale({kMuteScale * hudScale, kMuteScale * hudScale});
    m_MuteButton->SetVisible(false);
    m_MuteButton->SetSFX(Resource::SETTING_SFX);
    m_MuteButton->SetOnClickFunction([this]()
                                    {
                                        if (m_OnToggleMute)
                                        {
                                            m_OnToggleMute();
                                        }
                                    });
    addElement(m_MuteButton);

    m_MuteOverlay = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Game_Menu_Item_040), 97.0f);
    m_MuteOverlay->m_Transform.scale = {kMuteOverlayScale * hudScale, kMuteOverlayScale * hudScale};
    m_MuteOverlay->SetVisible(false);
    addElement(m_MuteOverlay);

    m_SoundButton = std::make_shared<Button>(Resource::Game_Menu_Item_063);
    m_SoundButton->SetZIndex(96.0f);
    m_SoundButton->SetScale({kSoundScale * hudScale, kSoundScale * hudScale});
    m_SoundButton->SetVisible(false);
    addElement(m_SoundButton);

    m_LevelTitle = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Level_Title_1_1), 96.0f);
    m_LevelTitle->m_Transform.scale = {kLevelTitleScale * hudScale, kLevelTitleScale * hudScale};
    m_LevelTitle->SetVisible(false);
    addElement(m_LevelTitle);
}

void PauseMenu::UpdateLayout(const glm::vec2 &cameraPos,
                             const glm::vec2 &viewportSize,
                             const float hudScale,
                             const float zoom)
{
    const glm::vec2 topLeftAnchor = cameraPos + glm::vec2{
                                                    -viewportSize.x * 0.5f / zoom,
                                                    viewportSize.y * 0.5f / zoom};

    PositionButton(m_CloseButton, topLeftAnchor, {kCloseOffsetX, kCloseOffsetY}, kCloseScale, hudScale, zoom);
    PositionButton(m_RestartButton, topLeftAnchor, {kRestartOffsetX, kRestartOffsetY}, kRestartScale, hudScale, zoom);
    PositionButton(m_MenuButton, topLeftAnchor, {kMenuOffsetX, kMenuOffsetY}, kMenuScale, hudScale, zoom);
    PositionButton(m_MuteButton, topLeftAnchor, {kMuteOffsetX, kMuteOffsetY}, kMuteScale, hudScale, zoom);
    PositionButton(m_SoundButton, topLeftAnchor, {kSoundOffsetX, kSoundOffsetY}, kSoundScale, hudScale, zoom);

    if (m_MuteOverlay)
    {
        m_MuteOverlay->m_Transform.translation = topLeftAnchor +
                                                 glm::vec2{
                                                     (kMuteOverlayOffsetX * hudScale) / zoom,
                                                     -(kMuteOverlayOffsetY * hudScale) / zoom};
        m_MuteOverlay->m_Transform.scale = {kMuteOverlayScale * hudScale / zoom, kMuteOverlayScale * hudScale / zoom};
    }

    if (m_Backdrop)
    {
        m_Backdrop->m_Transform.translation = cameraPos +
                                              glm::vec2{
                                                  -viewportSize.x * 0.5f / zoom + (kBackdropCenterOffsetX * hudScale) / zoom,
                                                  (kBackdropCenterOffsetY * hudScale) / zoom};
        m_Backdrop->m_Transform.scale = {
            (kBackdropWidth * hudScale) / zoom,
            viewportSize.y * kBackdropHeightRatio / zoom};
    }

    if (m_LevelTitle)
    {
        m_LevelTitle->m_Transform.translation = cameraPos +
                                                glm::vec2{
                                                    -viewportSize.x * 0.5f / zoom + (kLevelTitleOffsetX * hudScale) / zoom,
                                                    (kLevelTitleOffsetY * hudScale) / zoom};
        m_LevelTitle->m_Transform.scale = {kLevelTitleScale * hudScale / zoom, kLevelTitleScale * hudScale / zoom};
    }
}

void PauseMenu::SetVisible(const bool visible, const bool isMusicMuted, const int levelNumber)
{
    m_IsVisible = visible;
    RefreshLevelTitle(levelNumber);

    if (m_Backdrop)
    {
        m_Backdrop->SetVisible(visible);
    }
    if (m_CloseButton)
    {
        m_CloseButton->SetVisible(visible);
    }
    if (m_RestartButton)
    {
        m_RestartButton->SetVisible(visible);
    }
    if (m_MenuButton)
    {
        m_MenuButton->SetVisible(visible);
    }
    if (m_MuteButton)
    {
        m_MuteButton->SetVisible(visible);
    }
    if (m_MuteOverlay)
    {
        m_MuteOverlay->SetVisible(visible && isMusicMuted);
    }
    if (m_SoundButton)
    {
        m_SoundButton->SetVisible(visible);
    }
    if (m_LevelTitle)
    {
        m_LevelTitle->SetVisible(visible);
    }
}

void PauseMenu::SetButtonsInputEnabled(const bool enabled)
{
    if (m_CloseButton)
    {
        m_CloseButton->SetInputEnabled(enabled);
    }
    if (m_RestartButton)
    {
        m_RestartButton->SetInputEnabled(enabled);
    }
    if (m_MenuButton)
    {
        m_MenuButton->SetInputEnabled(enabled);
    }
    if (m_MuteButton)
    {
        m_MuteButton->SetInputEnabled(enabled);
    }
    if (m_SoundButton)
    {
        m_SoundButton->SetInputEnabled(enabled);
    }
}

void PauseMenu::SetMusicMuted(const bool isMusicMuted)
{
    if (m_MuteOverlay)
    {
        m_MuteOverlay->SetVisible(m_IsVisible && isMusicMuted);
    }
}

void PauseMenu::RefreshLevelTitle(const int levelNumber)
{
    if (!m_LevelTitle || levelNumber == m_CurrentLevelTitle)
    {
        return;
    }

    const std::string levelTitleResource = GetLevelTitleResource(levelNumber);
    if (!levelTitleResource.empty())
    {
        m_LevelTitle->SetDrawable(std::make_shared<Util::Image>(levelTitleResource));
        m_CurrentLevelTitle = levelNumber;
    }
}
