#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <string>

#include "Util/GameObject.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

class Character : public Util::GameObject {
public:
  explicit Character(const std::string &ImagePath);

  Character(const Character &) = delete;

  Character(Character &&) = delete;

  Character &operator=(const Character &) = delete;

  Character &operator=(Character &&) = delete;

  [[nodiscard]] const std::string &GetImagePath() const { return m_ImagePath; }

  [[nodiscard]] const glm::vec2 &GetPosition() const {
    return m_Transform.translation;
  }

  [[nodiscard]] bool GetVisibility() const { return m_Visible; }

  void SetImage(const std::string &ImagePath);

  void SetPosition(const glm::vec2 &Position) {
    m_Transform.translation = Position;
  }

  // TODO: Implement the collision detection
  [[nodiscard]] bool IfCollides(const std::shared_ptr<Character> &other) const {
    // (void) other;
    auto thisPos = GetPosition();
    auto otherPos = other->GetPosition();
    // Haven't updated to the size of the character
    auto thisWidth = GetSize().x;
    auto thisHeight = GetSize().y;
    auto otherWidth = other->GetSize().x;
    auto otherHeight = other->GetSize().y;
    if (abs(thisPos.x - otherPos.x) < (thisWidth / 2 + otherWidth / 2) &&
        abs(thisPos.y - otherPos.y) < (thisHeight / 2 + otherHeight / 2)) {
      return true;
    }
    return false;
  }
  [[nodiscard]] const glm::vec2 GetSize() const {
    auto size = this->GetScaledSize();
    return size;
  }

  [[nodiscard]] bool IsHovering(const glm::vec2 &mousePos) const {
    auto thisPos = GetPosition();
    auto thisSize = GetSize();
    return (mousePos.x >= thisPos.x - thisSize.x / 2 &&
            mousePos.x <= thisPos.x + thisSize.x / 2 &&
            mousePos.y >= thisPos.y - thisSize.y / 2 &&
            mousePos.y <= thisPos.y + thisSize.y / 2);
  }

  [[nodiscard]] bool IfClicked(const glm::vec2 &mousePos) const {
    return IsHovering(mousePos) &&
           Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB);
  }

    void UpdateHoverScale(const glm::vec2 &mousePos, float scaleAmount = 1.2f)
    {
        if (IsHovering(mousePos)) {
            m_Transform.scale = {scaleAmount, scaleAmount};
        } else {
            m_Transform.scale = {1.0f, 1.0f};
        }
    }
  // TODO: Add and implement more methods and properties as needed to finish
  // Giraffe Adventure.

private:
  void ResetPosition() { m_Transform.translation = {0, 0}; }

  std::string m_ImagePath;
};

#endif // CHARACTER_HPP
