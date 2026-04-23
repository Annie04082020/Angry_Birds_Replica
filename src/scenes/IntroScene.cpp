#include "IntroScene.hpp"

#include "BGM.hpp"
#include "IntroLayout.hpp"
#include "Resource.hpp"
#include "UILayout.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Time.hpp"
#include "Util/TransformUtils.hpp"
#include "SDL.h"

#include <cmath>

std::shared_ptr<IntroScene> IntroScene::Create()
{
    auto bg = std::make_shared<DynamicBackground>(Resource::MOVING_BG_IMAGE);
    struct Enabler : public IntroScene
    {
        Enabler(std::shared_ptr<DynamicBackground> bg) : IntroScene(bg) {}
    };
    return std::make_shared<Enabler>(bg);
}

IntroScene::IntroScene(std::shared_ptr<DynamicBackground> bg)
    : Scene(bg), m_movingBg(bg)
{
    SetVisible(true);
    SetZIndex(50);
    SetBGM(std::make_shared<BackgroundMusic>(Resource::TITLE_THEME));

    const glm::vec2 viewportSize = Util::GetViewportSize();
    const IntroLayout layout = IntroLayoutLoader::Load(Resource::INTRO_LAYOUT_DATA);
    m_settingButtonPosition = UILayout::PercentToWorldPosition(
        layout.settingButtonBase.xPercent, layout.settingButtonBase.yPercent, viewportSize);
    m_additionalButtonPosition = UILayout::PercentToWorldPosition(
        layout.additionalButtonBase.xPercent, layout.additionalButtonBase.yPercent, viewportSize);

    m_playbutton = std::make_shared<Button>(Resource::Play_Button);
    m_menuItemSpacing = layout.menuConfig.itemSpacing;
    m_menuInitialOffset = layout.menuConfig.initialOffset;
    m_menuAnimationDistance = layout.menuConfig.animationDistance;
    m_menuAnimationSpeed = layout.menuConfig.animationSpeed;
    m_playbutton->SetZIndex(50);
    m_playbutton->SetPosition(UILayout::PercentToWorldPosition(
        layout.play.xPercent, layout.play.yPercent, viewportSize));
    m_playbutton->SetScale({layout.play.scale, layout.play.scale});
    m_playbutton->SetVisible(true);
    m_playbutton->SetSFX(Resource::SETTING_SFX);
    m_playbutton->SetOnClickFunction([this]()
                                     {
        const bool canOpenLevelSelect = (m_onPlayClick == nullptr) || m_onPlayClick();
        if (canOpenLevelSelect)
        {
            HideExitPanel();
        } });

    m_exitbutton = std::make_shared<Button>(Resource::Exit_Button);
    m_exitbutton->SetZIndex(50);
    m_exitbutton->SetPosition(UILayout::PercentToWorldPosition(
        layout.exit.xPercent, layout.exit.yPercent, viewportSize));
    m_exitbutton->SetScale({layout.exit.scale, layout.exit.scale});
    m_exitbutton->SetVisible(true);
    m_exitbutton->SetSFX(Resource::SETTING_SFX);
    m_exitbutton->SetOnClickFunction([this]()
                                     {
        m_exitConfirm048->SetVisible(true);
        m_exitButton105->SetVisible(true);
        m_exitButton95->SetVisible(true);
        m_exitDialog->SetVisible(true);
        m_exitPanelVisible = true; });

    const float settingBaseScaleValue = layout.settingButtonBase.scale * layout.settingButtonBase.baseScale;
    const float settingOverlayScaleValue =
        layout.settingButtonOverlay.scale * layout.settingButtonOverlay.overlayScale;

    m_settingScale = {settingOverlayScaleValue, settingOverlayScaleValue};
    m_settingScaleHover = m_settingScale * layout.settingButtonOverlay.hoverScaleMultiplier;

    m_settingbutton = std::make_shared<Button>(Resource::Setting_Button_Base);
    m_settingbutton->SetZIndex(50);
    m_settingbutton->SetPosition(m_settingButtonPosition);
    m_settingbutton->SetScale({settingBaseScaleValue, settingBaseScaleValue});
    m_settingbutton->SetVisible(true);
    m_settingbutton->SetHoverScaleMultiplier(layout.settingButtonOverlay.hoverScaleMultiplier);
    m_settingbutton->SetSFX(Resource::SETTING_SFX);
    m_settingbutton->SetOnClickFunction([this]()
                                        {
        if (m_settingMenuOpen) {
            m_settingOverlayTargetRotation += 3.14159265f;
            m_settingMenuOpen = false;
            m_menuItemsAnimating = true;
        } else {
            m_settingOverlayTargetRotation -= 3.14159265f;
            m_settingMenuOpen = true;
            m_menuItemsAnimating = true;
        }
        m_settingOverlayIsAnimating = true; });

    m_settingOverlay = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Button_Overlay), 51);
    m_settingOverlay->m_Transform.translation = m_settingButtonPosition + glm::vec2{0.0f, 6.0f};
    m_settingOverlay->m_Transform.scale = m_settingScale;
    m_settingOverlay->SetVisible(true);

    const float additionalBaseScaleValue =
        layout.additionalButtonBase.scale * layout.additionalButtonBase.baseScale;
    const float additionalOverlayScaleValue =
        layout.additionalButtonOverlay.scale * layout.additionalButtonOverlay.overlayScale;

    m_additionalScale = {additionalOverlayScaleValue, additionalOverlayScaleValue};
    m_additionalScaleHover =
        m_additionalScale * layout.additionalButtonOverlay.hoverScaleMultiplier;

    m_additionalButton = std::make_shared<Button>(Resource::Additional_Button_Base);
    m_additionalButton->SetZIndex(50);
    m_additionalButton->SetPosition(m_additionalButtonPosition);
    m_additionalButton->SetScale({additionalBaseScaleValue, additionalBaseScaleValue});
    m_additionalButton->SetVisible(true);
    m_additionalButton->SetHoverScaleMultiplier(layout.additionalButtonOverlay.hoverScaleMultiplier);
    m_additionalButton->SetSFX(Resource::SETTING_SFX);
    m_additionalButton->SetOnClickFunction([this]()
                                           {
        if (m_additionalMenuOpen) {
            m_additionalOverlayTargetRotation += 3.14159265f;
            m_additionalMenuOpen = false;
            m_additionalMenuItemsAnimating = true;
        } else {
            m_additionalOverlayTargetRotation -= 3.14159265f;
            m_additionalMenuOpen = true;
            m_additionalMenuItemsAnimating = true;
        }
        m_additionalOverlayIsAnimating = true; });

    m_additionalButtonOverlay = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Button_Overlay), 51);
    m_additionalButtonOverlay->m_Transform.translation = m_additionalButtonPosition + glm::vec2{0.0f, 6.0f};
    m_additionalButtonOverlay->m_Transform.scale = m_additionalScale;
    m_additionalButtonOverlay->SetVisible(true);

    auto getGroupScaleMultiplier = [&layout](const std::string &groupId) -> float
    {
        if (groupId.empty())
        {
            return 1.0f;
        }
        auto it = layout.groups.find(groupId);
        if (it != layout.groups.end())
        {
            return it->second.scaleMultiplier;
        }
        return 1.0f;
    };

    auto applyMenuItemLayout = [&](const std::shared_ptr<Util::GameObject> &menuItem,
                                   const std::unordered_map<std::string, MenuItemConfig> &menuItems,
                                   const std::string &itemId,
                                   const glm::vec2 &basePosition)
    {
        const auto itemIt = menuItems.find(itemId);
        if (itemIt == menuItems.end())
        {
            menuItem->m_Transform.translation = basePosition;
            menuItem->m_Transform.scale = {1.0f, 1.0f};
            return;
        }

        const auto &item = itemIt->second;
        const float groupScale = getGroupScaleMultiplier(item.groupId);
        menuItem->m_Transform.translation = basePosition + glm::vec2{0.0f, item.relativeOffsetY * groupScale};
        menuItem->m_Transform.scale = {item.scale * groupScale, item.scale * groupScale};
    };

    m_menuItem043 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Menu_Item_043), 20);
    applyMenuItemLayout(m_menuItem043, layout.settingMenuItems.items, "043", m_settingButtonPosition);
    m_menuItem043->SetVisible(false);

    m_menuItem032 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Menu_Item_032), 21);
    applyMenuItemLayout(m_menuItem032, layout.settingMenuItems.items, "032", m_settingButtonPosition);
    m_menuItem032->SetVisible(false);

    m_menuItem017 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Menu_Item_017), 22);
    applyMenuItemLayout(m_menuItem017, layout.settingMenuItems.items, "017", m_settingButtonPosition);
    m_menuItem017->SetVisible(false);

    m_additionalMenuItem108 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Menu_Item_108), 20);
    applyMenuItemLayout(m_additionalMenuItem108, layout.additionalMenuItems.items, "108", m_additionalButtonPosition);
    m_additionalMenuItem108->SetVisible(false);

    m_additionalMenuItem006 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Menu_Item_006), 21);
    applyMenuItemLayout(m_additionalMenuItem006, layout.additionalMenuItems.items, "006", m_additionalButtonPosition);
    m_additionalMenuItem006->SetVisible(false);

    m_additionalMenuItem041 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Menu_Item_041), 22);
    applyMenuItemLayout(m_additionalMenuItem041, layout.additionalMenuItems.items, "041", m_additionalButtonPosition);
    m_additionalMenuItem041->SetVisible(false);

    m_exitConfirm048 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Confirm_048), 60);
    m_exitConfirm048->m_Transform.translation = UILayout::PercentToWorldPosition(
        layout.exitConfirm.xPercent, layout.exitConfirm.yPercent, viewportSize);
    m_exitConfirm048->m_Transform.scale = glm::vec2{layout.exitConfirm.scale, layout.exitConfirm.scale};
    m_exitConfirm048->SetVisible(false);

    m_exitButton105 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Button_105), 61);
    m_exitButton105->m_Transform.translation = UILayout::PercentToWorldPosition(
        layout.exitNo.xPercent, layout.exitNo.yPercent, viewportSize);
    m_exitButton105->m_Transform.scale = glm::vec2{layout.exitNo.scale, layout.exitNo.scale};
    m_exitButton105->SetVisible(false);

    m_exitButton95 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Button_95), 61);
    m_exitButton95->m_Transform.translation = UILayout::PercentToWorldPosition(
        layout.exitYes.xPercent, layout.exitYes.yPercent, viewportSize);
    m_exitButton95->m_Transform.scale = glm::vec2{layout.exitYes.scale, layout.exitYes.scale};
    m_exitButton95->SetVisible(false);

    m_exitDialog = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Dialog), 62);
    m_exitDialog->m_Transform.translation = UILayout::PercentToWorldPosition(
        layout.exitDialog.xPercent, layout.exitDialog.yPercent, viewportSize);
    m_exitDialog->m_Transform.scale = glm::vec2{layout.exitDialog.scale, layout.exitDialog.scale};
    m_exitDialog->SetVisible(false);

    AddElements(m_playbutton);
    AddElements(m_exitbutton);
    AddElements(m_settingbutton);
    AddElements(m_settingOverlay);
    AddElements(m_additionalButton);
    AddElements(m_additionalButtonOverlay);
    AddElements(m_menuItem043);
    AddElements(m_menuItem032);
    AddElements(m_menuItem017);
    AddElements(m_additionalMenuItem108);
    AddElements(m_additionalMenuItem006);
    AddElements(m_additionalMenuItem041);
    AddElements(m_exitConfirm048);
    AddElements(m_exitButton105);
    AddElements(m_exitButton95);
    AddElements(m_exitDialog);
}

void IntroScene::SetMenuVisible(const bool visible)
{
    m_playbutton->SetVisible(visible);
    m_exitbutton->SetVisible(visible);
    m_settingbutton->SetVisible(visible);
    m_settingOverlay->SetVisible(visible);
    m_additionalButton->SetVisible(visible);
    m_additionalButtonOverlay->SetVisible(visible);

    if (!visible)
    {
        m_settingMenuOpen = false;
        m_additionalMenuOpen = false;
        m_menuItemsAnimating = false;
        m_additionalMenuItemsAnimating = false;
        HideExitPanel();
    }

    m_menuItem043->SetVisible(visible && m_settingMenuOpen);
    m_menuItem032->SetVisible(visible && m_settingMenuOpen);
    m_menuItem017->SetVisible(visible && m_settingMenuOpen);
    m_additionalMenuItem108->SetVisible(visible && m_additionalMenuOpen);
    m_additionalMenuItem006->SetVisible(visible && m_additionalMenuOpen);
    m_additionalMenuItem041->SetVisible(visible && m_additionalMenuOpen);
}

void IntroScene::HideExitPanel()
{
    m_exitConfirm048->SetVisible(false);
    m_exitButton105->SetVisible(false);
    m_exitButton95->SetVisible(false);
    m_exitDialog->SetVisible(false);
    m_exitPanelVisible = false;
}

void IntroScene::Update()
{
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) && !m_exitPanelVisible && m_exitbutton->GetVisibility())
    {
        m_exitConfirm048->SetVisible(true);
        m_exitButton105->SetVisible(true);
        m_exitButton95->SetVisible(true);
        m_exitDialog->SetVisible(true);
        m_exitPanelVisible = true;
    }

    if (m_settingOverlayIsAnimating && m_settingOverlay)
    {
        float &rotation = m_settingOverlay->m_Transform.rotation;
        const float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        constexpr float rotationSpeedRadPerSec = 9.42478f;
        const float rotationStep = rotationSpeedRadPerSec * deltaTimeSec;

        const float remaining = m_settingOverlayTargetRotation - rotation;
        if (std::fabs(remaining) <= rotationStep)
        {
            rotation = m_settingOverlayTargetRotation;
            m_settingOverlayIsAnimating = false;
        }
        else
        {
            rotation += (remaining > 0 ? rotationStep : -rotationStep);
        }
    }

    if (m_additionalOverlayIsAnimating && m_additionalButtonOverlay)
    {
        float &rotation = m_additionalButtonOverlay->m_Transform.rotation;
        const float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        constexpr float rotationSpeedRadPerSec = 11.780975f;
        const float rotationStep = rotationSpeedRadPerSec * deltaTimeSec;

        const float remaining = m_additionalOverlayTargetRotation - rotation;
        if (std::fabs(remaining) <= rotationStep)
        {
            rotation = m_additionalOverlayTargetRotation;
            m_additionalOverlayIsAnimating = false;
        }
        else
        {
            rotation += (remaining > 0 ? rotationStep : -rotationStep);
        }
    }

    if (m_menuItemsAnimating && m_menuItem043 && m_menuItem032 && m_menuItem017)
    {
        const float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0f;

        if (m_settingMenuOpen)
        {
            glm::vec2 &pos043 = m_menuItem043->m_Transform.translation;
            glm::vec2 &pos032 = m_menuItem032->m_Transform.translation;
            glm::vec2 &pos017 = m_menuItem017->m_Transform.translation;
            const float moveDistance = m_menuAnimationSpeed * deltaTimeSec;
            const float targetY043 = m_settingButtonPosition.y + m_menuInitialOffset - m_menuItemSpacing * 2 + m_menuAnimationDistance;
            const float targetY032 = m_settingButtonPosition.y + m_menuInitialOffset - m_menuItemSpacing + m_menuAnimationDistance - 3.0f;
            const float targetY017 = m_settingButtonPosition.y + m_menuInitialOffset + m_menuAnimationDistance;

            if (pos043.y < targetY043)
            {
                pos043.y = glm::min(pos043.y + moveDistance, targetY043);
            }
            if (pos032.y < targetY032)
            {
                pos032.y = glm::min(pos032.y + moveDistance, targetY032);
            }
            if (pos017.y < targetY017)
            {
                pos017.y = glm::min(pos017.y + moveDistance, targetY017);
            }

            m_menuItem043->SetVisible(pos043.y > m_settingButtonPosition.y);
            m_menuItem032->SetVisible(pos032.y > m_settingButtonPosition.y);
            m_menuItem017->SetVisible(pos017.y > m_settingButtonPosition.y);

            if (std::fabs(pos043.y - targetY043) < 1.0f &&
                std::fabs(pos032.y - targetY032) < 1.0f &&
                std::fabs(pos017.y - targetY017) < 1.0f)
            {
                pos043.y = targetY043;
                pos032.y = targetY032;
                pos017.y = targetY017;
                m_menuItemsAnimating = false;
            }
        }
        else
        {
            glm::vec2 &pos043 = m_menuItem043->m_Transform.translation;
            glm::vec2 &pos032 = m_menuItem032->m_Transform.translation;
            glm::vec2 &pos017 = m_menuItem017->m_Transform.translation;
            const float moveDistance = m_menuAnimationSpeed * deltaTimeSec;
            const float targetY043 = m_settingButtonPosition.y + m_menuInitialOffset - m_menuItemSpacing * 2;
            const float targetY032 = m_settingButtonPosition.y + m_menuInitialOffset - m_menuItemSpacing;
            const float targetY017 = m_settingButtonPosition.y + m_menuInitialOffset;

            if (pos043.y > targetY043)
            {
                pos043.y = glm::max(pos043.y - moveDistance, targetY043);
            }
            if (pos032.y > targetY032)
            {
                pos032.y = glm::max(pos032.y - moveDistance, targetY032);
            }
            if (pos017.y > targetY017)
            {
                pos017.y = glm::max(pos017.y - moveDistance, targetY017);
            }

            m_menuItem043->SetVisible(pos043.y > m_settingButtonPosition.y);
            m_menuItem032->SetVisible(pos032.y > m_settingButtonPosition.y);
            m_menuItem017->SetVisible(pos017.y > m_settingButtonPosition.y);

            if (std::fabs(pos043.y - targetY043) < 1.0f &&
                std::fabs(pos032.y - targetY032) < 1.0f &&
                std::fabs(pos017.y - targetY017) < 1.0f)
            {
                pos043.y = targetY043;
                pos032.y = targetY032;
                pos017.y = targetY017;
                m_menuItemsAnimating = false;
            }
        }
    }

    if (m_additionalMenuItemsAnimating && m_additionalMenuItem108 && m_additionalMenuItem006 && m_additionalMenuItem041)
    {
        const float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        if (m_additionalMenuOpen)
        {
            glm::vec2 &pos108 = m_additionalMenuItem108->m_Transform.translation;
            glm::vec2 &pos006 = m_additionalMenuItem006->m_Transform.translation;
            glm::vec2 &pos041 = m_additionalMenuItem041->m_Transform.translation;
            const float moveDistance = m_menuAnimationSpeed * deltaTimeSec;
            const float targetY108 = m_additionalButtonPosition.y + m_menuInitialOffset - m_menuItemSpacing * 2 + m_menuAnimationDistance;
            const float targetY006 = m_additionalButtonPosition.y + m_menuInitialOffset - m_menuItemSpacing + m_menuAnimationDistance - 3.0f;
            const float targetY041 = m_additionalButtonPosition.y + m_menuInitialOffset + m_menuAnimationDistance;

            if (pos108.y < targetY108)
            {
                pos108.y = glm::min(pos108.y + moveDistance, targetY108);
            }
            if (pos006.y < targetY006)
            {
                pos006.y = glm::min(pos006.y + moveDistance, targetY006);
            }
            if (pos041.y < targetY041)
            {
                pos041.y = glm::min(pos041.y + moveDistance, targetY041);
            }

            m_additionalMenuItem108->SetVisible(pos108.y > m_additionalButtonPosition.y);
            m_additionalMenuItem006->SetVisible(pos006.y > m_additionalButtonPosition.y);
            m_additionalMenuItem041->SetVisible(pos041.y > m_additionalButtonPosition.y);

            if (std::fabs(pos108.y - targetY108) < 1.0f &&
                std::fabs(pos006.y - targetY006) < 1.0f &&
                std::fabs(pos041.y - targetY041) < 1.0f)
            {
                pos108.y = targetY108;
                pos006.y = targetY006;
                pos041.y = targetY041;
                m_additionalMenuItemsAnimating = false;
            }
        }
        else
        {
            glm::vec2 &pos108 = m_additionalMenuItem108->m_Transform.translation;
            glm::vec2 &pos006 = m_additionalMenuItem006->m_Transform.translation;
            glm::vec2 &pos041 = m_additionalMenuItem041->m_Transform.translation;
            const float moveDistance = m_menuAnimationSpeed * deltaTimeSec;
            const float targetY108 = m_additionalButtonPosition.y + m_menuInitialOffset - m_menuItemSpacing * 2;
            const float targetY006 = m_additionalButtonPosition.y + m_menuInitialOffset - m_menuItemSpacing;
            const float targetY041 = m_additionalButtonPosition.y + m_menuInitialOffset;

            if (pos108.y > targetY108)
            {
                pos108.y = glm::max(pos108.y - moveDistance, targetY108);
            }
            if (pos006.y > targetY006)
            {
                pos006.y = glm::max(pos006.y - moveDistance, targetY006);
            }
            if (pos041.y > targetY041)
            {
                pos041.y = glm::max(pos041.y - moveDistance, targetY041);
            }

            m_additionalMenuItem108->SetVisible(pos108.y > m_additionalButtonPosition.y);
            m_additionalMenuItem006->SetVisible(pos006.y > m_additionalButtonPosition.y);
            m_additionalMenuItem041->SetVisible(pos041.y > m_additionalButtonPosition.y);

            if (std::fabs(pos108.y - targetY108) < 1.0f &&
                std::fabs(pos006.y - targetY006) < 1.0f &&
                std::fabs(pos041.y - targetY041) < 1.0f)
            {
                pos108.y = targetY108;
                pos006.y = targetY006;
                pos041.y = targetY041;
                m_additionalMenuItemsAnimating = false;
            }
        }
    }

    const auto mousePos = Util::Input::GetCursorPosition();
    if (m_settingbutton && m_settingOverlay && m_settingbutton->IsHovering(mousePos) && m_settingbutton->GetVisibility())
    {
        m_settingOverlay->m_Transform.scale = m_settingScaleHover;
    }
    else if (m_settingOverlay)
    {
        m_settingOverlay->m_Transform.scale = m_settingScale;
    }

    if (m_additionalButton && m_additionalButtonOverlay && m_additionalButton->IsHovering(mousePos) && m_additionalButton->GetVisibility())
    {
        m_additionalButtonOverlay->m_Transform.scale = m_additionalScaleHover;
    }
    else if (m_additionalButtonOverlay)
    {
        m_additionalButtonOverlay->m_Transform.scale = m_additionalScale;
    }

    if (m_exitPanelVisible && m_exitButton95 && m_exitButton105)
    {
        const auto exitButton95Size = m_exitButton95->GetScaledSize();
        const auto exitButton105Size = m_exitButton105->GetScaledSize();
        const auto pos95 = m_exitButton95->m_Transform.translation;
        const auto pos105 = m_exitButton105->m_Transform.translation;

        if (mousePos.x >= pos95.x - exitButton95Size.x / 2 &&
            mousePos.x <= pos95.x + exitButton95Size.x / 2 &&
            mousePos.y >= pos95.y - exitButton95Size.y / 2 &&
            mousePos.y <= pos95.y + exitButton95Size.y / 2 &&
            Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB))
        {
            SDL_Event quitEvent;
            quitEvent.type = SDL_QUIT;
            SDL_PushEvent(&quitEvent);
        }

        if (mousePos.x >= pos105.x - exitButton105Size.x / 2 &&
            mousePos.x <= pos105.x + exitButton105Size.x / 2 &&
            mousePos.y >= pos105.y - exitButton105Size.y / 2 &&
            mousePos.y <= pos105.y + exitButton105Size.y / 2 &&
            Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB))
        {
            HideExitPanel();
        }
    }

    m_movingBg->Update();
    Scene::Update();
}
