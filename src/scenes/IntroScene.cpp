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
    // Store menu configuration from JSON
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
        m_additionalMenuItem041->SetVisible(false); });

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

    // scale: group multiplier, baseScale/overlayScale: per-layer multipliers.
    const float settingBaseScaleValue = layout.settingButtonBase.scale * layout.settingButtonBase.baseScale;
    const float settingOverlayScaleValue =
        layout.settingButtonOverlay.scale * layout.settingButtonOverlay.overlayScale;

    // 使用底圖與覆層分開的按鈕：底圖保留光影設計，覆層（齒輪）單獨旋轉
    m_settingScale = {settingOverlayScaleValue, settingOverlayScaleValue};
    m_settingScaleHover = m_settingScale * layout.settingButtonOverlay.hoverScaleMultiplier;

    m_settingbutton = std::make_shared<Button>(Resource::Setting_Button_Base);
    m_settingbutton->SetZIndex(50);
    m_settingbutton->SetPosition(m_settingButtonPosition);
    m_settingbutton->SetScale({settingBaseScaleValue, settingBaseScaleValue});
    m_settingbutton->SetVisible(true);
    m_settingbutton->SetHoverScaleMultiplier(layout.settingButtonOverlay.hoverScaleMultiplier); // 啟用 hover 縮放
    m_settingbutton->SetSFX(Resource::SETTING_SFX);
    m_settingbutton->SetOnClickFunction([this]()
                                        {
        if (m_settingMenuOpen) {
            // 關閉菜單：逆時針旋轉 180 度
            m_settingOverlayTargetRotation += 3.14159265f;
            m_settingMenuOpen = false;
            m_menuItemsAnimating = true;  // 啟動動畫回到原位置
            // 菜單項保持可見，直到動畫完成
        } else {
            // 展開菜單：順時針旋轉 180 度
            m_settingOverlayTargetRotation -= 3.14159265f;
            m_settingMenuOpen = true;
            m_menuItemsAnimating = true;
            // 菜單項visibility由動畫邏輯控制
        }
        m_settingOverlayIsAnimating = true; });

    // 建立覆層（齒輪），比底圖 z-index 高一層
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
    m_additionalButton->SetScale(
        {additionalBaseScaleValue, additionalBaseScaleValue});
    m_additionalButton->SetVisible(true);
    m_additionalButton->SetHoverScaleMultiplier(
        layout.additionalButtonOverlay.hoverScaleMultiplier);
    m_additionalButton->SetSFX(Resource::SETTING_SFX);
    m_additionalButton->SetOnClickFunction([this]()
                                           {
        // Handle menu opening/closing
        if (m_additionalMenuOpen) {
            // Close menu: rotate back
            m_additionalOverlayTargetRotation += 3.14159265f;
            m_additionalMenuOpen = false;
            m_additionalMenuItemsAnimating = true;
        } else {
            // Open menu: rotate 180 degrees
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
            return 1.0f;
        auto it = layout.groups.find(groupId);
        if (it != layout.groups.end())
            return it->second.scaleMultiplier;
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

    // Setting menu items - apply JSON configuration and group scaling
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

    // Additional menu items - apply JSON configuration and group scaling
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

    // Exit confirm panel: 048 in the center with 105 (left) and 95 (right) below
    m_exitConfirm048 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Confirm_048), 60);
    m_exitConfirm048->m_Transform.translation = UILayout::PercentToWorldPosition(
        layout.exitConfirm.xPercent, layout.exitConfirm.yPercent, viewportSize);
    m_exitConfirm048->m_Transform.scale =
        glm::vec2{layout.exitConfirm.scale, layout.exitConfirm.scale};
    m_exitConfirm048->SetVisible(false);

    m_exitButton105 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Button_105), 61);
    m_exitButton105->m_Transform.translation = UILayout::PercentToWorldPosition(
        layout.exitNo.xPercent, layout.exitNo.yPercent, viewportSize);
    m_exitButton105->m_Transform.scale =
        glm::vec2{layout.exitNo.scale, layout.exitNo.scale};
    m_exitButton105->SetVisible(false);

    m_exitButton95 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Button_95), 61);
    m_exitButton95->m_Transform.translation = UILayout::PercentToWorldPosition(
        layout.exitYes.xPercent, layout.exitYes.yPercent, viewportSize);
    m_exitButton95->m_Transform.scale =
        glm::vec2{layout.exitYes.scale, layout.exitYes.scale};
    m_exitButton95->SetVisible(false);

    m_exitDialog = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Dialog), 62);
    m_exitDialog->m_Transform.translation = UILayout::PercentToWorldPosition(
        layout.exitDialog.xPercent, layout.exitDialog.yPercent, viewportSize);
    m_exitDialog->m_Transform.scale =
        glm::vec2{layout.exitDialog.scale, layout.exitDialog.scale};
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

void IntroScene::Update()
{
    // Handle ESC key to show exit confirmation
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) && !m_exitPanelVisible)
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
        float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        constexpr float rotationSpeedRadPerSec = 3.0f * glm::pi<float>(); // 3π rad/sec
        float rotationStep = rotationSpeedRadPerSec * deltaTimeSec;

        float remaining = m_settingOverlayTargetRotation - rotation;
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
        float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        constexpr float rotationSpeedRadPerSec = 3.75f * glm::pi<float>();
        float rotationStep = rotationSpeedRadPerSec * deltaTimeSec;

        float remaining = m_additionalOverlayTargetRotation - rotation;
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
    if (m_settingbutton && m_settingOverlay && m_settingbutton->IsHovering(mousePos))
    {
        m_settingOverlay->m_Transform.scale = m_settingScaleHover;
    }
    else if (m_settingOverlay)
    {
        m_settingOverlay->m_Transform.scale = m_settingScale;
    }

    // Hover scale for additional button overlay (096) when hovering base (068)
    if (m_additionalButton && m_additionalButtonOverlay && m_additionalButton->IsHovering(mousePos))
    {
        m_additionalButtonOverlay->m_Transform.scale = m_additionalScaleHover;
    }
    else if (m_additionalButtonOverlay)
    {
        m_additionalButtonOverlay->m_Transform.scale = m_additionalScale;
    }

    // Handle exit panel button clicks (095 and 105)
    if (m_exitPanelVisible && m_exitButton95 && m_exitButton105)
    {
        auto exitButton95Size = m_exitButton95->GetScaledSize();
        auto exitButton105Size = m_exitButton105->GetScaledSize();
        auto pos95 = m_exitButton95->m_Transform.translation;
        auto pos105 = m_exitButton105->m_Transform.translation;

        // Check if 095 (right button) is clicked - exit game
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

        // Check if 105 (left button) is clicked - continue game
        if (mousePos.x >= pos105.x - exitButton105Size.x / 2 &&
            mousePos.x <= pos105.x + exitButton105Size.x / 2 &&
            mousePos.y >= pos105.y - exitButton105Size.y / 2 &&
            mousePos.y <= pos105.y + exitButton105Size.y / 2 &&
            Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB))
        {
            // Cancel exit - hide exit panel
            m_exitConfirm048->SetVisible(false);
            m_exitButton105->SetVisible(false);
            m_exitButton95->SetVisible(false);
            m_exitDialog->SetVisible(false);
            m_exitPanelVisible = false;
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
