#ifndef Button_HPP
#define Button_HPP

#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "SoundEffect.hpp"
#include "Util/GameObject.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/TransformUtils.hpp"

class Button : public Util::GameObject
{
public:
  explicit Button(const std::string &ImagePath);

  Button(const Button &) = delete;

  Button(Button &&) = delete;

  Button &operator=(const Button &) = delete;

  Button &operator=(Button &&) = delete;

  [[nodiscard]] const std::string &GetImagePath() const { return m_ImagePath; }

  [[nodiscard]] const glm::vec2 &GetPosition() const
  {
    return m_Transform.translation;
  }

  [[nodiscard]] bool GetVisibility() const { return m_Visible; }

  void SetImage(const std::string &ImagePath);

  void SetPosition(const glm::vec2 &Position)
  {
    m_Transform.translation = Position;
  }

  void SetScale(const glm::vec2 &Scale)
  {
    m_baseScale = Scale;
    m_Transform.scale = Scale;
  }

  void SetRotation(float Rotation) { m_Transform.rotation = Rotation; }

  [[nodiscard]] const glm::vec2 GetSize() const
  {
    auto size = this->GetScaledSize();
    return size;
  }

  [[nodiscard]] bool IsHovering(const glm::vec2 &mousePos) const
  {
    const float zoom = Util::GetCameraZoom();
    const glm::vec2 cameraPos = Util::GetCameraPosition();
    const glm::vec2 mouseWorldPos = mousePos / zoom + cameraPos;
    auto thisPos = GetPosition();
    auto thisSize = GetSize();
    return (mouseWorldPos.x >= thisPos.x - thisSize.x / 2 &&
            mouseWorldPos.x <= thisPos.x + thisSize.x / 2 &&
            mouseWorldPos.y >= thisPos.y - thisSize.y / 2 &&
            mouseWorldPos.y <= thisPos.y + thisSize.y / 2);
  }

  void SetOnClickFunction(std::function<void()> onClick)
  {
    m_OnClick = onClick;
  }

  void SetHoverScaleMultiplier(float multiplier)
  {
    m_hoverMultiplier = multiplier;
  }

  void SetInputEnabled(bool enabled)
  {
    m_InputEnabled = enabled;
  }

  void Update() override
  {
    if (!m_Visible)
    {
      m_IsPressed = Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB);
      return;
    }

    auto mousePos = Util::Input::GetCursorPosition();

    if (IsHovering(mousePos))
    {
      m_Transform.scale = m_baseScale * m_hoverMultiplier;
    }
    else
    {
      m_Transform.scale = m_baseScale;
    }

    if (!m_InputEnabled)
    {
      m_IsPressed = Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB);
      return;
    }

    const bool isClickedNow =
        IsHovering(mousePos) && Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB);

    if (isClickedNow && !m_IsPressed)
    {
      if (m_SFX)
      {
        m_SFX->Play_SFX();
      }
      if (m_OnClick)
      {
        m_OnClick();
      }
    }
    m_IsPressed = Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB);
  }

  void SetSFX(const std::string &SFXPath)
  {
    m_SFXPath = SFXPath;
    m_SFX = std::make_shared<SoundEffect>(m_SFXPath);
  }

  void Init() override { m_Transform.scale = m_baseScale; }

private:
  void ResetPosition() { m_Transform.translation = {0, 0}; }

  std::string m_ImagePath;
  std::string m_SFXPath;
  std::shared_ptr<SoundEffect> m_SFX;
  glm::vec2 m_baseScale = {1.0f, 1.0f};
  float m_hoverMultiplier = 1.125f;
  bool m_IsPressed = false;
  bool m_InputEnabled = true;
  std::function<void()> m_OnClick = nullptr;
};

#endif // Button_HPP
