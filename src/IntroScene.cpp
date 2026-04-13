#include "IntroScene.hpp"
#include "BGM.hpp"
#include "JsonParseUtils.hpp"
#include "Resource.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Time.hpp"
#include "Util/TransformUtils.hpp"

#include <cmath>
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace
{
    struct IntroLayoutPercents
    {
        // Neutral fallback values; tuned layout should live in intro_layout.json.
        float playXPercent = 50.0f;
        float playYPercent = 50.0f;
        float playScale = 1.0f;
        float exitXPercent = 50.0f;
        float exitYPercent = 50.0f;
        float exitScale = 1.0f;
        float settingXPercent = 50.0f;
        float settingYPercent = 50.0f;
        float settingScale = 1.0f;
        float settingBaseScale = 1.0f;
        float settingOverlayScale = 1.0f;
        float settingHoverScaleMultiplier = 1.125f;
        float additionalXPercent = 50.0f;
        float additionalYPercent = 50.0f;
        float additionalScale = 1.0f;
        float additionalBaseScale = 1.0f;
        float additionalOverlayScale = 1.0f;
        float additionalHoverScaleMultiplier = 1.125f;
        float exitConfirmXPercent = 50.0f;
        float exitConfirmYPercent = 50.0f;
        float exitYesXPercent = 60.0f;
        float exitYesYPercent = 54.0f;
        float exitNoXPercent = 40.0f;
        float exitNoYPercent = 54.0f;
        float exitDialogXPercent = 50.0f;
        float exitDialogYPercent = 47.0f;
    };

    void ParseButtonSection(const std::string &json, const std::string &section,
                            float &xPercent, float &yPercent, float &scale)
    {
        std::string sectionJson;
        if (!JsonParseUtils::ExtractObjectContent(json, section, sectionJson))
        {
            return;
        }

        xPercent = JsonParseUtils::ExtractFloat(sectionJson, "xPercent", xPercent);
        yPercent = JsonParseUtils::ExtractFloat(sectionJson, "yPercent", yPercent);
        scale = JsonParseUtils::ExtractFloat(sectionJson, "scale", scale);
    }

    void ParseCompositeButtonSection(const std::string &json,
                                     const std::string &section,
                                     float &xPercent, float &yPercent,
                                     float &groupScale, float &baseScale,
                                     float &overlayScale,
                                     float &hoverScaleMultiplier)
    {
        std::string sectionJson;
        if (!JsonParseUtils::ExtractObjectContent(json, section, sectionJson))
        {
            return;
        }

        xPercent = JsonParseUtils::ExtractFloat(sectionJson, "xPercent", xPercent);
        yPercent = JsonParseUtils::ExtractFloat(sectionJson, "yPercent", yPercent);
        groupScale = JsonParseUtils::ExtractFloat(sectionJson, "scale", groupScale);
        baseScale = JsonParseUtils::ExtractFloat(sectionJson, "baseScale", baseScale);
        overlayScale = JsonParseUtils::ExtractFloat(sectionJson, "overlayScale", overlayScale);
        hoverScaleMultiplier = JsonParseUtils::ExtractFloat(sectionJson, "hoverScaleMultiplier", hoverScaleMultiplier);
    }

    void ParseButtonSection(const std::string &json, const std::string &section,
                            float &xPercent, float &yPercent)
    {
        std::string sectionJson;
        if (!JsonParseUtils::ExtractObjectContent(json, section, sectionJson))
        {
            return;
        }

        xPercent = JsonParseUtils::ExtractFloat(sectionJson, "xPercent", xPercent);
        yPercent = JsonParseUtils::ExtractFloat(sectionJson, "yPercent", yPercent);
    }

    IntroLayoutPercents LoadIntroLayoutPercents()
    {
        IntroLayoutPercents layout;

        std::ifstream layoutFile(Resource::INTRO_LAYOUT_DATA);
        if (!layoutFile)
        {
            return layout;
        }

        std::stringstream buffer;
        buffer << layoutFile.rdbuf();
        const std::string layoutJson = buffer.str();

        ParseButtonSection(layoutJson, "play", layout.playXPercent,
                           layout.playYPercent, layout.playScale);
        ParseButtonSection(layoutJson, "exit", layout.exitXPercent,
                           layout.exitYPercent, layout.exitScale);
        ParseCompositeButtonSection(layoutJson, "setting", layout.settingXPercent,
                                    layout.settingYPercent, layout.settingScale,
                                    layout.settingBaseScale,
                                    layout.settingOverlayScale,
                                    layout.settingHoverScaleMultiplier);
        ParseCompositeButtonSection(layoutJson, "additional", layout.additionalXPercent,
                                    layout.additionalYPercent,
                                    layout.additionalScale,
                                    layout.additionalBaseScale,
                                    layout.additionalOverlayScale,
                                    layout.additionalHoverScaleMultiplier);
        ParseButtonSection(layoutJson, "exitConfirm", layout.exitConfirmXPercent,
                           layout.exitConfirmYPercent);
        ParseButtonSection(layoutJson, "exitYes", layout.exitYesXPercent,
                           layout.exitYesYPercent);
        ParseButtonSection(layoutJson, "exitNo", layout.exitNoXPercent,
                           layout.exitNoYPercent);
        ParseButtonSection(layoutJson, "exitDialog", layout.exitDialogXPercent,
                           layout.exitDialogYPercent);

        return layout;
    }

    glm::vec2 PercentToWorldPosition(float xPercent, float yPercent,
                                     const glm::vec2 &viewportSize)
    {
        const float x = (xPercent / 100.0f - 0.5f) * viewportSize.x;
        const float y = (0.5f - yPercent / 100.0f) * viewportSize.y;
        return {x, y};
    }
} // namespace

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

    const glm::vec2 viewportSize = Util::GetViewportSize();
    const IntroLayoutPercents layout = LoadIntroLayoutPercents();
    m_settingButtonPosition = PercentToWorldPosition(
        layout.settingXPercent, layout.settingYPercent, viewportSize);
    m_additionalButtonPosition = PercentToWorldPosition(
        layout.additionalXPercent, layout.additionalYPercent, viewportSize);

    m_playbutton = std::make_shared<Button>(Resource::Play_Button);
    m_playbutton->SetZIndex(50);
    m_playbutton->SetPosition(PercentToWorldPosition(
        layout.playXPercent, layout.playYPercent, viewportSize));
    m_playbutton->SetScale({layout.playScale, layout.playScale});
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
        } });

    m_exitbutton = std::make_shared<Button>(Resource::Exit_Button);
    m_exitbutton->SetZIndex(50);
    m_exitbutton->SetPosition(PercentToWorldPosition(
        layout.exitXPercent, layout.exitYPercent, viewportSize));
    m_exitbutton->SetScale({layout.exitScale, layout.exitScale});
    m_exitbutton->SetVisible(true);
    m_exitbutton->SetSFX(Resource::SETTING_SFX);
    m_exitbutton->SetOnClickFunction([this]()
                                     {
        m_exitConfirm048->SetVisible(true);
        m_exitButton105->SetVisible(true);
        m_exitButton95->SetVisible(true);
        m_exitDialog->SetVisible(true);
        m_exitPanelVisible = true;
        m_bird->SetVisible(true); });

    // scale: group multiplier, baseScale/overlayScale: per-layer multipliers.
    const float settingBaseScaleValue = layout.settingScale * layout.settingBaseScale;
    const float settingOverlayScaleValue =
        layout.settingScale * layout.settingOverlayScale;

    // 使用底圖與覆層分開的按鈕：底圖保留光影設計，覆層（齒輪）單獨旋轉
    m_settingScale = {settingOverlayScaleValue, settingOverlayScaleValue};
    m_settingScaleHover = m_settingScale * layout.settingHoverScaleMultiplier;

    m_settingbutton = std::make_shared<Button>(Resource::Setting_Button_Base);
    m_settingbutton->SetZIndex(50);
    m_settingbutton->SetPosition(m_settingButtonPosition);
    m_settingbutton->SetScale({settingBaseScaleValue, settingBaseScaleValue});
    m_settingbutton->SetVisible(true);
    m_settingbutton->SetHoverScaleMultiplier(layout.settingHoverScaleMultiplier); // 啟用 hover 縮放
    m_settingbutton->SetSFX(Resource::SETTING_SFX);
    m_settingbutton->SetOnClickFunction([this]()
                                        {
        m_bird->SetVisible(true);
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
        layout.additionalScale * layout.additionalBaseScale;
    const float additionalOverlayScaleValue =
        layout.additionalScale * layout.additionalOverlayScale;

    m_additionalScale = {additionalOverlayScaleValue, additionalOverlayScaleValue};
    m_additionalScaleHover =
        m_additionalScale * layout.additionalHoverScaleMultiplier;

    m_additionalButton = std::make_shared<Button>(Resource::Additional_Button_Base);
    m_additionalButton->SetZIndex(50);
    m_additionalButton->SetPosition(m_additionalButtonPosition);
    m_additionalButton->SetScale(
        {additionalBaseScaleValue, additionalBaseScaleValue});
    m_additionalButton->SetVisible(true);
    m_additionalButton->SetHoverScaleMultiplier(
        layout.additionalHoverScaleMultiplier);
    m_additionalButton->SetSFX(Resource::SETTING_SFX);
    m_additionalButton->SetOnClickFunction([this]()
                                           {
        m_bird->SetVisible(true);
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

    // Initialize menu items (043 at bottom, 032 in middle, 017 at top)
    // Initialize menu items vertically stacked (spacing 70px, starting below setting button)
    constexpr float menuItemSpacing = 70.0f;
    constexpr float initialOffset = -5.0f; // Start below setting button

    m_menuItem043 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Menu_Item_043), 20);
    m_menuItem043->m_Transform.translation = m_settingButtonPosition + glm::vec2{0.0f, initialOffset - menuItemSpacing * 2};
    m_menuItem043->SetVisible(false);

    m_menuItem032 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Menu_Item_032), 21);
    m_menuItem032->m_Transform.translation = m_settingButtonPosition + glm::vec2{0.0f, initialOffset - menuItemSpacing - 3.0f}; // +10 to prevent overlap with setting button
    m_menuItem032->SetVisible(false);

    m_menuItem017 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Setting_Menu_Item_017), 22);
    m_menuItem017->m_Transform.translation = m_settingButtonPosition + glm::vec2{0.0f, initialOffset};
    m_menuItem017->SetVisible(false);

    // Initialize additional button menu items (108 at bottom, 006 in middle, 041 at top)
    m_additionalMenuItem108 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Menu_Item_108), 20);
    m_additionalMenuItem108->m_Transform.translation = m_additionalButtonPosition + glm::vec2{0.0f, initialOffset - menuItemSpacing * 2};
    m_additionalMenuItem108->SetVisible(false);

    m_additionalMenuItem006 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Menu_Item_006), 21);
    m_additionalMenuItem006->m_Transform.translation = m_additionalButtonPosition + glm::vec2{0.0f, initialOffset - menuItemSpacing - 3.0f};
    m_additionalMenuItem006->SetVisible(false);

    m_additionalMenuItem041 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Additional_Menu_Item_041), 22);
    m_additionalMenuItem041->m_Transform.translation = m_additionalButtonPosition + glm::vec2{0.0f, initialOffset};
    m_additionalMenuItem041->SetVisible(false);

    // Exit confirm panel: 048 in the center with 105 (left) and 95 (right) below
    m_exitConfirm048 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Confirm_048), 60);
    m_exitConfirm048->m_Transform.translation = PercentToWorldPosition(
        layout.exitConfirmXPercent, layout.exitConfirmYPercent, viewportSize);
    m_exitConfirm048->m_Transform.scale = glm::vec2{3.0f, 3.0f};
    m_exitConfirm048->SetVisible(false);

    m_exitButton105 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Button_105), 61);
    m_exitButton105->m_Transform.translation = PercentToWorldPosition(
        layout.exitNoXPercent, layout.exitNoYPercent, viewportSize);
    m_exitButton105->SetVisible(false);

    m_exitButton95 = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Button_95), 61);
    m_exitButton95->m_Transform.translation = PercentToWorldPosition(
        layout.exitYesXPercent, layout.exitYesYPercent, viewportSize);
    m_exitButton95->SetVisible(false);

    m_exitDialog = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(Resource::Exit_Dialog), 62);
    m_exitDialog->m_Transform.translation = PercentToWorldPosition(
        layout.exitDialogXPercent, layout.exitDialogYPercent, viewportSize);
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
    constexpr float menuItemSpacing = 70.0f;
    constexpr float initialOffset = -50.0f;

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
        constexpr float menuAnimationSpeed = 400.0f; // pixels per second
        constexpr float targetMoveDistance = 300.0f; // move up 300px total

        if (m_settingMenuOpen)
        {
            // Move all items upward together
            glm::vec2 &pos043 = m_menuItem043->m_Transform.translation;
            glm::vec2 &pos032 = m_menuItem032->m_Transform.translation;
            glm::vec2 &pos017 = m_menuItem017->m_Transform.translation;

            float moveDistance = menuAnimationSpeed * deltaTimeSec;

            // Target: move up by targetMoveDistance (Y increases), maintain spacing
            float targetY043 = m_settingButtonPosition.y + initialOffset - menuItemSpacing * 2 + targetMoveDistance;
            float targetY032 = m_settingButtonPosition.y + initialOffset - menuItemSpacing + targetMoveDistance - 3.0f; // +10 to prevent overlap with setting button
            float targetY017 = m_settingButtonPosition.y + initialOffset + targetMoveDistance;

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

            // Show items only when they float above setting button
            m_menuItem043->SetVisible(pos043.y > m_settingButtonPosition.y);
            m_menuItem032->SetVisible(pos032.y > m_settingButtonPosition.y);
            m_menuItem017->SetVisible(pos017.y > m_settingButtonPosition.y);

            // Check if all items reached their targets
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
            // Move items back to original positions
            glm::vec2 &pos043 = m_menuItem043->m_Transform.translation;
            glm::vec2 &pos032 = m_menuItem032->m_Transform.translation;
            glm::vec2 &pos017 = m_menuItem017->m_Transform.translation;

            float moveDistance = menuAnimationSpeed * deltaTimeSec;

            float targetY043 = m_settingButtonPosition.y + initialOffset - menuItemSpacing * 2;
            float targetY032 = m_settingButtonPosition.y + initialOffset - menuItemSpacing; // +5 to prevent overlap with setting button
            float targetY017 = m_settingButtonPosition.y + initialOffset;

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

            // Hide items only when they go below setting button
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

    // Animate additional menu items opening/closing (similar to setting menu)
    if (m_additionalMenuItemsAnimating && m_additionalMenuItem108 && m_additionalMenuItem006 && m_additionalMenuItem041)
    {
        float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        constexpr float menuAnimationSpeed = 400.0f; // pixels per second
        constexpr float targetMoveDistance = 300.0f; // move up 300px total

        if (m_additionalMenuOpen)
        {
            // Move all items upward together
            glm::vec2 &pos108 = m_additionalMenuItem108->m_Transform.translation;
            glm::vec2 &pos006 = m_additionalMenuItem006->m_Transform.translation;
            glm::vec2 &pos041 = m_additionalMenuItem041->m_Transform.translation;

            float moveDistance = menuAnimationSpeed * deltaTimeSec;

            // Target: move up by targetMoveDistance (Y increases), maintain spacing
            float targetY108 = m_additionalButtonPosition.y + initialOffset - menuItemSpacing * 2 + targetMoveDistance;
            float targetY006 = m_additionalButtonPosition.y + initialOffset - menuItemSpacing + targetMoveDistance - 3.0f;
            float targetY041 = m_additionalButtonPosition.y + initialOffset + targetMoveDistance;

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

            // Show items only when they float above additional button
            m_additionalMenuItem108->SetVisible(pos108.y > m_additionalButtonPosition.y);
            m_additionalMenuItem006->SetVisible(pos006.y > m_additionalButtonPosition.y);
            m_additionalMenuItem041->SetVisible(pos041.y > m_additionalButtonPosition.y);

            // Check if all items reached their targets
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
            // Move items back to original positions
            glm::vec2 &pos108 = m_additionalMenuItem108->m_Transform.translation;
            glm::vec2 &pos006 = m_additionalMenuItem006->m_Transform.translation;
            glm::vec2 &pos041 = m_additionalMenuItem041->m_Transform.translation;

            float moveDistance = menuAnimationSpeed * deltaTimeSec;

            float targetY108 = m_additionalButtonPosition.y + initialOffset - menuItemSpacing * 2;
            float targetY006 = m_additionalButtonPosition.y + initialOffset - menuItemSpacing;
            float targetY041 = m_additionalButtonPosition.y + initialOffset;

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

            // Hide items only when they go below additional button
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
            // Exit game - need to implement proper exit
            std::exit(0);
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
