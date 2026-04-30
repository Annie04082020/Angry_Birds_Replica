#include "ui/AnimatedButton.hpp"
#include "Util/AnimationUtils.hpp"
#include "Util/Time.hpp"

AnimatedButton::AnimatedButton(const std::shared_ptr<Button> &baseButton,
                               const std::shared_ptr<Util::GameObject> &overlay)
    : m_Base(baseButton), m_Overlay(overlay)
{
}

void AnimatedButton::Init()
{
    if (m_Base)
        m_Base->Init();
    if (m_Overlay)
        m_Overlay->Init();
}

void AnimatedButton::Update()
{
    if (!m_Base)
        return;

    // Base handles hover scaling and click handling on its own
    m_Base->Update();

    // Overlay hover scaling when base is hovered
    auto mousePos = Util::Input::GetCursorPosition();
    if (m_Overlay)
    {
        if (m_Base->IsHovering(mousePos))
            m_Overlay->m_Transform.scale = m_OverlayHoverScale;
        else
            m_Overlay->m_Transform.scale = m_OverlayBaseScale;

        if (m_IsOverlayAnimating)
        {
            float &rotation = m_Overlay->m_Transform.rotation;
            float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
            constexpr float rotationSpeed = 3.0f * glm::pi<float>();
            m_IsOverlayAnimating = Util::AnimationUtils::StepRotateTowards(rotation, m_OverlayTarget, deltaTimeSec, rotationSpeed);
        }
    }
}

void AnimatedButton::SetOnClickFunction(std::function<void()> fn)
{
    if (m_Base)
        m_Base->SetOnClickFunction(fn);
}

void AnimatedButton::SetSFX(const std::string &sfxPath)
{
    if (m_Base)
        m_Base->SetSFX(sfxPath);
}

void AnimatedButton::SetPosition(const glm::vec2 &pos)
{
    if (m_Base)
        m_Base->SetPosition(pos);
    if (m_Overlay)
        m_Overlay->m_Transform.translation = pos + glm::vec2{0.0f, 6.0f};
}

void AnimatedButton::SetScale(const glm::vec2 &scale)
{
    if (m_Base)
        m_Base->SetScale(scale);
}

void AnimatedButton::SetVisible(bool v)
{
    if (m_Base)
        m_Base->SetVisible(v);
    if (m_Overlay)
        m_Overlay->SetVisible(v);
}

bool AnimatedButton::IsHovering(const glm::vec2 &mousePos) const
{
    if (!m_Base)
        return false;
    return m_Base->IsHovering(mousePos);
}

void AnimatedButton::SetOverlayTargetRotation(float target)
{
    m_OverlayTarget = target;
    m_IsOverlayAnimating = true;
}

void AnimatedButton::SetOverlayScales(const glm::vec2 &baseScale, const glm::vec2 &hoverScale)
{
    m_OverlayBaseScale = baseScale;
    m_OverlayHoverScale = hoverScale;
    if (m_Overlay)
        m_Overlay->m_Transform.scale = baseScale;
}
