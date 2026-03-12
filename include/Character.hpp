#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <string>

#include "Util/GameObject.hpp"

class Character : public Util::GameObject
{
public:
    explicit Character(const std::string &ImagePath);

    Character(const Character &) = delete;

    Character(Character &&) = delete;

    Character &operator=(const Character &) = delete;

    Character &operator=(Character &&) = delete;

    [[nodiscard]] const std::string &GetImagePath() const { return m_ImagePath; }

    [[nodiscard]] const glm::vec2 &GetPosition() const { return m_Transform.translation; }

    [[nodiscard]] bool GetVisibility() const { return m_Visible; }

    void SetImage(const std::string &ImagePath);

    void SetPosition(const glm::vec2 &Position) { m_Transform.translation = Position; }

    // TODO: Implement the collision detection
    [[nodiscard]] bool IfCollides(const std::shared_ptr<Character> &other) const
    {
        // (void) other;
        auto thisPos = GetPosition();
        auto otherPos = other->GetPosition();
        // Haven't updated to the size of the character
        auto thisWidth = GetSize().x;
        auto thisHeight = GetSize().y;
        auto otherWidth = other->GetSize().x;
        auto otherHeight = other->GetSize().y;
        if (abs(thisPos.x - otherPos.x) < (thisWidth / 2 + otherWidth / 2) && abs(thisPos.y - otherPos.y) < (thisHeight / 2 + otherHeight / 2))
        {
            return true;
        }
        return false;
    }
    [[nodiscard]] const glm::vec2 GetSize() const
    {
        auto size = this->GetScaledSize();
        return size;
    }
    // TODO: Add and implement more methods and properties as needed to finish Giraffe Adventure.

private:
    void ResetPosition() { m_Transform.translation = {0, 0}; }

    std::string m_ImagePath;
};

#endif // CHARACTER_HPP
