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
#include "Util/AnimationUtils.hpp"
#include "Util/LayoutUtils.hpp"
#include "Util/MouseUtils.hpp"

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
        const bool transitionSucceeded = (m_onPlayClick == nullptr) || m_onPlayClick();
        if (!transitionSucceeded)
        {
            return;
        }

        // Stop title BGM once the game transition succeeds.
        StopBGM();

        m_playbutton->SetVisible(false);
        m_settingbutton->SetVisible(false);
        m_settingOverlay->SetVisible(false);
        m_additionalButton->SetVisible(false);
        m_additionalButtonOverlay->SetVisible(false);
        m_exitbutton->SetVisible(false);
        // Hide menu items
        m_menuItem043->SetVisible(false);
        m_menuItem032->SetVisible(false);
        m_menuItem017->SetVisible(false);
        m_additionalMenuItem108->SetVisible(false);
        m_additionalMenuItem006->SetVisible(false);
        m_additionalMenuItem041->SetVisible(false); 
        HideExitPanel(); });

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
            // 關閉菜單：逆時針旋轉 180 度
            if (m_settingAnimated)
                m_settingAnimated->SetOverlayTargetRotation(m_settingOverlay->m_Transform.rotation + 3.14159265f);
            m_settingMenuOpen = false;
            m_menuItemsAnimating = true;
        } else {
            // 展開菜單：順時針旋轉 180 度
            if (m_settingAnimated)
                m_settingAnimated->SetOverlayTargetRotation(m_settingOverlay->m_Transform.rotation - 3.14159265f);
            m_settingMenuOpen = true;
            m_menuItemsAnimating = true;
        }
        if (m_settingAnimated) { /* animation started via SetOverlayTargetRotation */ } });

    m_settingOverlay = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Button_Overlay), 51);
    m_settingOverlay->m_Transform.translation = m_settingButtonPosition + glm::vec2{0.0f, 6.0f};
    m_settingOverlay->m_Transform.scale = m_settingScale;
    m_settingOverlay->SetVisible(true);

    // Compose AnimatedButton for setting button + overlay
    m_settingAnimated = std::make_shared<AnimatedButton>(m_settingbutton, m_settingOverlay);
    m_settingAnimated->SetOverlayScales(m_settingScale, m_settingScaleHover);

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
            // Close menu: rotate back
            if (m_additionalAnimated)
                m_additionalAnimated->SetOverlayTargetRotation(m_additionalButtonOverlay->m_Transform.rotation + 3.14159265f);
            m_additionalMenuOpen = false;
            m_additionalMenuItemsAnimating = true;
        } else {
            // Open menu: rotate 180 degrees
            if (m_additionalAnimated)
                m_additionalAnimated->SetOverlayTargetRotation(m_additionalButtonOverlay->m_Transform.rotation - 3.14159265f);
            m_additionalMenuOpen = true;
            m_additionalMenuItemsAnimating = true;
        }
        if (m_additionalAnimated) { /* animation started via SetOverlayTargetRotation */ } });

    m_additionalButtonOverlay = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Button_Overlay), 51);
    m_additionalButtonOverlay->m_Transform.translation = m_additionalButtonPosition + glm::vec2{0.0f, 6.0f};
    m_additionalButtonOverlay->m_Transform.scale = m_additionalScale;
    m_additionalButtonOverlay->SetVisible(true);

    // Compose AnimatedButton for additional button + overlay
    m_additionalAnimated = std::make_shared<AnimatedButton>(m_additionalButton, m_additionalButtonOverlay);
    m_additionalAnimated->SetOverlayScales(m_additionalScale, m_additionalScaleHover);

    // Use LayoutUtils helpers for group scaling and menu item layout.

    m_menuItem043 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Menu_Item_043), 20);
    Util::LayoutUtils::ApplyMenuItemLayout(m_menuItem043, layout.settingMenuItems.items, "043", m_settingButtonPosition, layout);
    m_menuItem043->SetVisible(false);

    m_menuItem032 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Menu_Item_032), 21);
    Util::LayoutUtils::ApplyMenuItemLayout(m_menuItem032, layout.settingMenuItems.items, "032", m_settingButtonPosition, layout);
    m_menuItem032->SetVisible(false);

    m_menuItem017 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Menu_Item_017), 22);
    Util::LayoutUtils::ApplyMenuItemLayout(m_menuItem017, layout.settingMenuItems.items, "017", m_settingButtonPosition, layout);
    m_menuItem017->SetVisible(false);

    m_additionalMenuItem108 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Menu_Item_108), 20);
    Util::LayoutUtils::ApplyMenuItemLayout(m_additionalMenuItem108, layout.additionalMenuItems.items, "108", m_additionalButtonPosition, layout);
    m_additionalMenuItem108->SetVisible(false);

    m_additionalMenuItem006 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Menu_Item_006), 21);
    Util::LayoutUtils::ApplyMenuItemLayout(m_additionalMenuItem006, layout.additionalMenuItems.items, "006", m_additionalButtonPosition, layout);
    m_additionalMenuItem006->SetVisible(false);

    m_additionalMenuItem041 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Menu_Item_041), 22);
    Util::LayoutUtils::ApplyMenuItemLayout(m_additionalMenuItem041, layout.additionalMenuItems.items, "041", m_additionalButtonPosition, layout);
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

    // Let AnimatedButton manage overlay rotation and hover scaling
    if (m_settingAnimated)
        m_settingAnimated->Update();
    if (m_additionalAnimated)
        m_additionalAnimated->Update();

    // Animate menu items opening/closing
    if (m_menuItemsAnimating)
    {
        std::vector<std::shared_ptr<Util::GameObject>> settingItems = {m_menuItem043, m_menuItem032, m_menuItem017};
        AnimateMenuItems(settingItems, m_settingButtonPosition, m_settingMenuOpen, m_menuItemsAnimating);
    }

    // Animate additional menu items opening/closing
    if (m_additionalMenuItemsAnimating)
    {
        std::vector<std::shared_ptr<Util::GameObject>> additionalItems = {m_additionalMenuItem108, m_additionalMenuItem006, m_additionalMenuItem041};
        AnimateMenuItems(additionalItems, m_additionalButtonPosition, m_additionalMenuOpen, m_additionalMenuItemsAnimating);
    }
    auto mousePos = Util::Input::GetCursorPosition();

    if (m_exitPanelVisible && m_exitButton95 && m_exitButton105)
    {
        const auto exitButton95Size = m_exitButton95->GetScaledSize();
        const auto exitButton105Size = m_exitButton105->GetScaledSize();
        const auto pos95 = m_exitButton95->m_Transform.translation;
        const auto pos105 = m_exitButton105->m_Transform.translation;

        // Check if 095 (right button) is clicked - exit game
        if (Util::MouseUtils::IsClickedOver(mousePos, m_exitButton95, Util::Keycode::MOUSE_LB))
        {
            SDL_Event quitEvent;
            quitEvent.type = SDL_QUIT;
            SDL_PushEvent(&quitEvent);
        }

        // Check if 105 (left button) is clicked - continue game
        if (Util::MouseUtils::IsClickedOver(mousePos, m_exitButton105, Util::Keycode::MOUSE_LB))
        {
            HideExitPanel();
        }
    }

    m_movingBg->Update();
    Scene::Update();
}

void IntroScene::AnimateMenuItems(
    std::vector<std::shared_ptr<Util::GameObject>> &items,
    const glm::vec2 &basePosition,
    bool menuOpen,
    bool &isAnimating)
{
    if (!isAnimating || items.empty())
        return;

    float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
    float moveDistance = m_menuAnimationSpeed * deltaTimeSec;
    size_t numItems = items.size();

    if (menuOpen)
    {
        // Move items upward
        bool allReached = true;
        for (size_t i = 0; i < numItems; ++i)
        {
            if (!items[i])
                continue;

            glm::vec2 &pos = items[i]->m_Transform.translation;
            float baseOffset = -m_menuItemSpacing * (float)(numItems - 1 - i);
            float extraAdjust = (i == 1) ? -3.0f : 0.0f;
            float targetY = basePosition.y + m_menuInitialOffset + baseOffset + m_menuAnimationDistance + extraAdjust;

            if (pos.y < targetY)
            {
                pos.y = glm::min(pos.y + moveDistance, targetY);
                allReached = false;
            }

            items[i]->SetVisible(pos.y > basePosition.y);
        }

        if (allReached)
            isAnimating = false;
    }
    else
    {
        // Move items back to original positions
        bool allReached = true;
        for (size_t i = 0; i < numItems; ++i)
        {
            if (!items[i])
                continue;

            glm::vec2 &pos = items[i]->m_Transform.translation;
            float baseOffset = -m_menuItemSpacing * (float)(numItems - 1 - i);
            float extraAdjust = (i == 1) ? -3.0f : 0.0f;
            float targetY = basePosition.y + m_menuInitialOffset + baseOffset + extraAdjust;

            if (pos.y > targetY)
            {
                pos.y = glm::max(pos.y - moveDistance, targetY);
                allReached = false;
            }

            items[i]->SetVisible(pos.y > basePosition.y);
        }

        if (allReached)
            isAnimating = false;
    }
}
