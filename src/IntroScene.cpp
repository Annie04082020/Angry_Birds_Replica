#include "IntroScene.hpp"
#include "BGM.hpp"
#include "Resource.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Time.hpp"
#include <cmath>
#include <cstdlib>

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

    m_bird = std::make_shared<Character>(Resource::BIRD_R);
    m_bird->SetVisible(false);

    m_playbutton = std::make_shared<Button>(Resource::Play_Button);
    m_playbutton->SetZIndex(50);
    m_playbutton->SetVisible(true);
    m_playbutton->SetSFX(Resource::SETTING_SFX);
    m_playbutton->SetOnClickFunction([this]()
                                     {
        m_playbutton->SetVisible(false);
        m_settingbutton->SetVisible(false);
        m_settingOverlay->SetVisible(false);
        m_additionalButton->SetVisible(false);
        m_additionalButtonOverlay->SetVisible(false);
        m_exitbutton->SetVisible(false);
        m_bird->SetVisible(true);
        if (m_onPlayClick) {
            m_onPlayClick();
        }
    });

    m_exitbutton = std::make_shared<Button>(Resource::Exit_Button);
    m_exitbutton->SetZIndex(50);
    m_exitbutton->SetPosition({-520.0f, -300.0f});
    m_exitbutton->SetVisible(true);
    m_exitbutton->SetSFX(Resource::SETTING_SFX);
    m_exitbutton->SetOnClickFunction([this]()
                                     {
        m_exitConfirm048->SetVisible(true);
        m_exitButton105->SetVisible(true);
        m_exitButton95->SetVisible(true);
        m_exitDialog->SetVisible(true);
        m_exitPanelVisible = true;
        m_bird->SetVisible(true); 
    });

    constexpr glm::vec2 settingPosition = {520.0f, -300.0f};

    m_settingbutton = std::make_shared<Button>(Resource::Setting_Button_Base);
    m_settingbutton->SetZIndex(50);
    m_settingbutton->SetPosition(settingPosition);
    m_settingbutton->SetScale(m_settingScale);
    m_settingbutton->SetVisible(true);
    m_settingbutton->SetHoverScaleMultiplier(1.125f);  // 啟用 hover 縮放
    m_settingbutton->SetSFX(Resource::SETTING_SFX);
    m_settingbutton->SetOnClickFunction([this]() {
        m_bird->SetVisible(true);
        if (m_settingMenuOpen) {
            // 關閉菜單：逆時針旋轉 180 度
            m_settingOverlayTargetRotation -= 3.14159265f;
            m_settingMenuOpen = false;
            m_menuItemsAnimating = true;  // 啟動動畫回到原位置
            // 菜單項保持可見，直到動畫完成
        } else {
            // 展開菜單：順時針旋轉 180 度
            m_settingOverlayTargetRotation += 3.14159265f;
            m_settingMenuOpen = true;
            m_menuItemsAnimating = true;
            // 菜單項visibility由動畫邏輯控制
        }
        m_settingOverlayIsAnimating = true;
    });

    m_settingOverlay = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Button_Overlay), 51);
    m_settingOverlay->m_Transform.translation = settingPosition + glm::vec2{0.0f, 6.0f};
    m_settingOverlay->m_Transform.scale = m_settingScale;
    m_settingOverlay->SetVisible(true);

    constexpr glm::vec2 additionalPosition = {650.0f, -300.0f};
    
    m_additionalButton = std::make_shared<Button>(Resource::Additional_Button_Base);
    m_additionalButton->SetZIndex(50);
    m_additionalButton->SetPosition(additionalPosition);
    m_additionalButton->SetScale(glm::vec2{1.0f, 1.0f});
    m_additionalButton->SetVisible(true);
    m_additionalButton->SetHoverScaleMultiplier(1.125f);
    m_additionalButton->SetSFX(Resource::SETTING_SFX);
    m_additionalButton->SetOnClickFunction([this]() {
        m_bird->SetVisible(true);
        // Handle menu opening/closing
        if (m_additionalMenuOpen) {
            // Close menu: rotate back
            m_additionalOverlayTargetRotation -= 3.14159265f;
            m_additionalMenuOpen = false;
            m_additionalMenuItemsAnimating = true;
        } else {
            // Open menu: rotate 180 degrees
            m_additionalOverlayTargetRotation += 3.14159265f;
            m_additionalMenuOpen = true;
            m_additionalMenuItemsAnimating = true;
        }
        m_additionalOverlayIsAnimating = true;
    });

    m_additionalButtonOverlay = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Button_Overlay), 51);
    m_additionalButtonOverlay->m_Transform.translation = additionalPosition + glm::vec2{0.0f, 6.0f};
    m_additionalButtonOverlay->m_Transform.scale = m_additionalScale;
    m_additionalButtonOverlay->SetVisible(true);

    // Initialize menu items (043 at bottom, 032 in middle, 017 at top)
    // Initialize menu items vertically stacked (spacing 70px, starting below setting button)
    constexpr float menuItemSpacing = 70.0f;
    constexpr float initialOffset = -5.0f;  // Start below setting button
    
    m_menuItem043 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Menu_Item_043), 20);
    m_menuItem043->m_Transform.translation = settingPosition + glm::vec2{0.0f, initialOffset - menuItemSpacing * 2};
    m_menuItem043->SetVisible(false);

    m_menuItem032 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Menu_Item_032), 21);
    m_menuItem032->m_Transform.translation = settingPosition + glm::vec2{0.0f, initialOffset - menuItemSpacing -3.0f}; // +10 to prevent overlap with setting button
    m_menuItem032->SetVisible(false);

    m_menuItem017 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Menu_Item_017), 22);
    m_menuItem017->m_Transform.translation = settingPosition + glm::vec2{0.0f, initialOffset};
    m_menuItem017->SetVisible(false);

    // Initialize additional button menu items (108 at bottom, 006 in middle, 041 at top)
    m_additionalMenuItem108 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Menu_Item_108), 20);
    m_additionalMenuItem108->m_Transform.translation = additionalPosition + glm::vec2{0.0f, initialOffset - menuItemSpacing * 2};
    m_additionalMenuItem108->SetVisible(false);

    m_additionalMenuItem006 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Menu_Item_006), 21);
    m_additionalMenuItem006->m_Transform.translation = additionalPosition + glm::vec2{0.0f, initialOffset - menuItemSpacing - 3.0f};
    m_additionalMenuItem006->SetVisible(false);

    m_additionalMenuItem041 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Menu_Item_041), 22);
    m_additionalMenuItem041->m_Transform.translation = additionalPosition + glm::vec2{0.0f, initialOffset};
    m_additionalMenuItem041->SetVisible(false);

    // Exit confirm panel: 048 in the center with 105 (left) and 95 (right) below
    m_exitConfirm048 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Confirm_048), 60);
    m_exitConfirm048->m_Transform.translation = glm::vec2{0.0f, 0.0f};
    m_exitConfirm048->m_Transform.scale = glm::vec2{3.0f, 3.0f};
    m_exitConfirm048->SetVisible(false);

    m_exitButton105 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Button_105), 61);
    m_exitButton105->m_Transform.translation = glm::vec2{-250.0f, -60.0f};
    m_exitButton105->SetVisible(false);

    m_exitButton95 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Button_95), 61);
    m_exitButton95->m_Transform.translation = glm::vec2{250.0f, -60.0f};
    m_exitButton95->SetVisible(false);

    m_exitDialog = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Dialog), 62);
    m_exitDialog->m_Transform.translation = glm::vec2{0.0f, 35.0f};
    m_exitDialog->m_Transform.scale = glm::vec2{0.35f, 0.35f};
    m_exitDialog->SetVisible(false);

    AddElements(m_bird);
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
    constexpr glm::vec2 settingPosition = {520.0f, -300.0f};
    constexpr glm::vec2 additionalPosition = {650.0f, -300.0f};
    constexpr float menuItemSpacing = 70.0f;
    constexpr float initialOffset = -50.0f;
    
    // Handle ESC key to show exit confirmation
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) && !m_exitPanelVisible) {
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
        constexpr float rotationSpeedRadPerSec = 9.42478f; // 3π rad/sec
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
        float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 800.0f;
        constexpr float rotationSpeedRadPerSec = 9.42478f; // 3π rad/sec
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
    if (m_menuItemsAnimating && m_menuItem043 && m_menuItem032 && m_menuItem017)
    {
        float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        constexpr float menuAnimationSpeed = 400.0f;  // pixels per second
        constexpr float targetMoveDistance = 300.0f;  // move up 300px total
        
        if (m_settingMenuOpen) {
            // Move all items upward together
            glm::vec2 &pos043 = m_menuItem043->m_Transform.translation;
            glm::vec2 &pos032 = m_menuItem032->m_Transform.translation;
            glm::vec2 &pos017 = m_menuItem017->m_Transform.translation;
            
            float moveDistance = menuAnimationSpeed * deltaTimeSec;
            
            // Target: move up by targetMoveDistance (Y increases), maintain spacing
            float targetY043 = settingPosition.y + initialOffset - menuItemSpacing * 2 + targetMoveDistance;
            float targetY032 = settingPosition.y + initialOffset - menuItemSpacing + targetMoveDistance - 3.0f; // +10 to prevent overlap with setting button
            float targetY017 = settingPosition.y + initialOffset + targetMoveDistance;
            
            if (pos043.y < targetY043) {
                pos043.y = glm::min(pos043.y + moveDistance, targetY043);
            }
            if (pos032.y < targetY032) {
                pos032.y = glm::min(pos032.y + moveDistance, targetY032);
            }
            if (pos017.y < targetY017) {
                pos017.y = glm::min(pos017.y + moveDistance, targetY017);
            }
            
            // Show items only when they float above setting button
            m_menuItem043->SetVisible(pos043.y > settingPosition.y);
            m_menuItem032->SetVisible(pos032.y > settingPosition.y);
            m_menuItem017->SetVisible(pos017.y > settingPosition.y);
            
            // Check if all items reached their targets
            if (std::fabs(pos043.y - targetY043) < 1.0f &&
                std::fabs(pos032.y - targetY032) < 1.0f &&
                std::fabs(pos017.y - targetY017) < 1.0f) {
                pos043.y = targetY043;
                pos032.y = targetY032;
                pos017.y = targetY017;
                m_menuItemsAnimating = false;
            }
        } else {
            // Move items back to original positions
            glm::vec2 &pos043 = m_menuItem043->m_Transform.translation;
            glm::vec2 &pos032 = m_menuItem032->m_Transform.translation;
            glm::vec2 &pos017 = m_menuItem017->m_Transform.translation;
            
            float moveDistance = menuAnimationSpeed * deltaTimeSec;
            
            float targetY043 = settingPosition.y + initialOffset - menuItemSpacing * 2;
            float targetY032 = settingPosition.y + initialOffset - menuItemSpacing; // +5 to prevent overlap with setting button
            float targetY017 = settingPosition.y + initialOffset;
            
            if (pos043.y > targetY043) {
                pos043.y = glm::max(pos043.y - moveDistance, targetY043);
            }
            if (pos032.y > targetY032) {
                pos032.y = glm::max(pos032.y - moveDistance, targetY032);
            }
            if (pos017.y > targetY017) {
                pos017.y = glm::max(pos017.y - moveDistance, targetY017);
            }
            
            // Hide items only when they go below setting button
            m_menuItem043->SetVisible(pos043.y > settingPosition.y);
            m_menuItem032->SetVisible(pos032.y > settingPosition.y);
            m_menuItem017->SetVisible(pos017.y > settingPosition.y);
            
            if (std::fabs(pos043.y - targetY043) < 1.0f &&
                std::fabs(pos032.y - targetY032) < 1.0f &&
                std::fabs(pos017.y - targetY017) < 1.0f) {
                pos043.y = targetY043;
                pos032.y = targetY032;
                pos017.y = targetY017;
                m_menuItemsAnimating = false;
            }
        }
    }

    // Animate additional menu items opening/closing (similar to setting menu)
    if (m_additionalMenuItemsAnimating && m_additionalMenuItem108 && m_additionalMenuItem006 && m_additionalMenuItem041)
    {
        float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        constexpr float menuAnimationSpeed = 400.0f;  // pixels per second
        constexpr float targetMoveDistance = 300.0f;  // move up 300px total
        
        if (m_additionalMenuOpen) {
            // Move all items upward together
            glm::vec2 &pos108 = m_additionalMenuItem108->m_Transform.translation;
            glm::vec2 &pos006 = m_additionalMenuItem006->m_Transform.translation;
            glm::vec2 &pos041 = m_additionalMenuItem041->m_Transform.translation;
            
            float moveDistance = menuAnimationSpeed * deltaTimeSec;
            
            // Target: move up by targetMoveDistance (Y increases), maintain spacing
            float targetY108 = additionalPosition.y + initialOffset - menuItemSpacing * 2 + targetMoveDistance;
            float targetY006 = additionalPosition.y + initialOffset - menuItemSpacing + targetMoveDistance - 3.0f;
            float targetY041 = additionalPosition.y + initialOffset + targetMoveDistance;
            
            if (pos108.y < targetY108) {
                pos108.y = glm::min(pos108.y + moveDistance, targetY108);
            }
            if (pos006.y < targetY006) {
                pos006.y = glm::min(pos006.y + moveDistance, targetY006);
            }
            if (pos041.y < targetY041) {
                pos041.y = glm::min(pos041.y + moveDistance, targetY041);
            }
            
            // Show items only when they float above additional button
            m_additionalMenuItem108->SetVisible(pos108.y > additionalPosition.y);
            m_additionalMenuItem006->SetVisible(pos006.y > additionalPosition.y);
            m_additionalMenuItem041->SetVisible(pos041.y > additionalPosition.y);
            
            // Check if all items reached their targets
            if (std::fabs(pos108.y - targetY108) < 1.0f &&
                std::fabs(pos006.y - targetY006) < 1.0f &&
                std::fabs(pos041.y - targetY041) < 1.0f) {
                pos108.y = targetY108;
                pos006.y = targetY006;
                pos041.y = targetY041;
                m_additionalMenuItemsAnimating = false;
            }
        } else {
            // Move items back to original positions
            glm::vec2 &pos108 = m_additionalMenuItem108->m_Transform.translation;
            glm::vec2 &pos006 = m_additionalMenuItem006->m_Transform.translation;
            glm::vec2 &pos041 = m_additionalMenuItem041->m_Transform.translation;
            
            float moveDistance = menuAnimationSpeed * deltaTimeSec;
            
            float targetY108 = additionalPosition.y + initialOffset - menuItemSpacing * 2;
            float targetY006 = additionalPosition.y + initialOffset - menuItemSpacing;
            float targetY041 = additionalPosition.y + initialOffset;
            
            if (pos108.y > targetY108) {
                pos108.y = glm::max(pos108.y - moveDistance, targetY108);
            }
            if (pos006.y > targetY006) {
                pos006.y = glm::max(pos006.y - moveDistance, targetY006);
            }
            if (pos041.y > targetY041) {
                pos041.y = glm::max(pos041.y - moveDistance, targetY041);
            }
            
            // Hide items only when they go below additional button
            m_additionalMenuItem108->SetVisible(pos108.y > additionalPosition.y);
            m_additionalMenuItem006->SetVisible(pos006.y > additionalPosition.y);
            m_additionalMenuItem041->SetVisible(pos041.y > additionalPosition.y);
            
            if (std::fabs(pos108.y - targetY108) < 1.0f &&
                std::fabs(pos006.y - targetY006) < 1.0f &&
                std::fabs(pos041.y - targetY041) < 1.0f) {
                pos108.y = targetY108;
                pos006.y = targetY006;
                pos041.y = targetY041;
                m_additionalMenuItemsAnimating = false;
            }
        }
    }
    auto mousePos = Util::Input::GetCursorPosition();
    if (m_settingbutton && m_settingOverlay && m_settingbutton->IsHovering(mousePos)) {
        m_settingOverlay->m_Transform.scale = m_settingScaleHover;
    } else if (m_settingOverlay) {
        m_settingOverlay->m_Transform.scale = m_settingScale;
    }

    // Hover scale for additional button overlay (096) when hovering base (068)
    if (m_additionalButton && m_additionalButtonOverlay && m_additionalButton->IsHovering(mousePos)) {
        m_additionalButtonOverlay->m_Transform.scale = m_additionalScaleHover;
    } else if (m_additionalButtonOverlay) {
        m_additionalButtonOverlay->m_Transform.scale = m_additionalScale;
    }

    // Handle exit panel button clicks (095 and 105)
    if (m_exitPanelVisible && m_exitButton95 && m_exitButton105) {
        auto exitButton95Size = m_exitButton95->GetScaledSize();
        auto exitButton105Size = m_exitButton105->GetScaledSize();
        auto pos95 = m_exitButton95->m_Transform.translation;
        auto pos105 = m_exitButton105->m_Transform.translation;
        
        // Check if 095 (right button) is clicked - exit game
        if (mousePos.x >= pos95.x - exitButton95Size.x / 2 &&
            mousePos.x <= pos95.x + exitButton95Size.x / 2 &&
            mousePos.y >= pos95.y - exitButton95Size.y / 2 &&
            mousePos.y <= pos95.y + exitButton95Size.y / 2 &&
            Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            // Exit game - need to implement proper exit
            std::exit(0);
        }
        
        // Check if 105 (left button) is clicked - continue game
        if (mousePos.x >= pos105.x - exitButton105Size.x / 2 &&
            mousePos.x <= pos105.x + exitButton105Size.x / 2 &&
            mousePos.y >= pos105.y - exitButton105Size.y / 2 &&
            mousePos.y <= pos105.y + exitButton105Size.y / 2 &&
            Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
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
