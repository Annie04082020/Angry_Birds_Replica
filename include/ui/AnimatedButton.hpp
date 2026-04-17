#pragma once

#include "../PTSD/include/Util/GameObject.hpp"
#include "ui/Button.hpp"
#include <memory>

class AnimatedButton
{
public:
    AnimatedButton(const std::shared_ptr<Button> &baseButton,
                   const std::shared_ptr<Util::GameObject> &overlay);

    void Init();
    void Update();

    void SetOnClickFunction(std::function<void()> fn);
    void SetSFX(const std::string &sfxPath);
    void SetPosition(const glm::vec2 &pos);
    void SetScale(const glm::vec2 &scale);
    void SetVisible(bool v);
    bool IsHovering(const glm::vec2 &mousePos) const;

    void SetOverlayTargetRotation(float target);
    void SetOverlayScales(const glm::vec2 &baseScale, const glm::vec2 &hoverScale);

private:
    std::shared_ptr<Button> m_Base;
    std::shared_ptr<Util::GameObject> m_Overlay;
    float m_OverlayTarget = 0.0f;
    bool m_IsOverlayAnimating = false;
    glm::vec2 m_OverlayBaseScale = {1.0f, 1.0f};
    glm::vec2 m_OverlayHoverScale = {1.0f, 1.0f};
};
