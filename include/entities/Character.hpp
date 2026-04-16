#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <string>

#include "Util/GameObject.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

class Character : public Util::GameObject
{
public:
  enum class EntityKind
  {
    Unknown,
    Bird,
    Pig,
    Environment,
    Slingshot
  };

  enum class MaterialType
  {
    None,
    Flesh,
    Wood,
    Stone,
    Glass,
    Ice
  };

  struct PhysicsState
  {
    float mass = 1.0f;
    glm::vec2 velocity = {0.0f, 0.0f};
    float angularVelocity = 0.0f;
    glm::vec2 centerOfMassOffset = {0.0f, 0.0f};
    float inertia = 1.0f;
    bool isStatic = false;
  };

  explicit Character(const std::string &ImagePath);

  Character(const Character &) = delete;

  Character(Character &&) = delete;

  Character &operator=(const Character &) = delete;

  Character &operator=(Character &&) = delete;

  [[nodiscard]] const std::string &GetImagePath() const { return m_ImagePath; }

  [[nodiscard]] const glm::vec2 &GetPosition() const
  {
    return m_Transform.translation;
  }

  [[nodiscard]] bool GetVisibility() const { return m_Visible; }

  [[nodiscard]] const PhysicsState &GetPhysicsState() const
  {
    return m_PhysicsState;
  }

  void SetPhysicsState(const PhysicsState &physicsState);

  [[nodiscard]] float GetMass() const { return m_PhysicsState.mass; }

  void SetMass(float mass);

  [[nodiscard]] float GetInertia() const { return m_PhysicsState.inertia; }

  void SetInertia(float inertia);

  [[nodiscard]] const glm::vec2 &GetVelocity() const
  {
    return m_PhysicsState.velocity;
  }

  void SetVelocity(const glm::vec2 &velocity)
  {
    m_PhysicsState.velocity = velocity;
  }

  [[nodiscard]] float GetAngularVelocity() const
  {
    return m_PhysicsState.angularVelocity;
  }

  void SetAngularVelocity(float angularVelocity)
  {
    m_PhysicsState.angularVelocity = angularVelocity;
  }

  [[nodiscard]] const glm::vec2 &GetCenterOfMassOffset() const
  {
    return m_PhysicsState.centerOfMassOffset;
  }

  void SetCenterOfMassOffset(const glm::vec2 &offset)
  {
    m_PhysicsState.centerOfMassOffset = offset;
  }

  [[nodiscard]] glm::vec2 GetCenterOfMassWorldPosition() const
  {
    return m_Transform.translation + m_PhysicsState.centerOfMassOffset;
  }

  [[nodiscard]] bool IsStatic() const { return m_PhysicsState.isStatic; }

  void SetStatic(bool isStatic) { m_PhysicsState.isStatic = isStatic; }

  [[nodiscard]] EntityKind GetEntityKind() const { return m_EntityKind; }

  void SetEntityKind(EntityKind entityKind) { m_EntityKind = entityKind; }

  [[nodiscard]] MaterialType GetMaterialType() const { return m_MaterialType; }

  void SetMaterialType(MaterialType materialType)
  {
    m_MaterialType = materialType;
  }

  void SetImage(const std::string &ImagePath);

  void SetPosition(const glm::vec2 &Position)
  {
    m_Transform.translation = Position;
  }

  void SetScale(const glm::vec2 &Scale) { m_Transform.scale = Scale; }

  void SetRotation(float Rotation) { m_Transform.rotation = Rotation; }

  // Apply one Euler integration step and write the result to transform.
  void IntegratePhysics(float deltaTimeSeconds);

  // Collision detection: first AABB broad-phase, then OBB/SAT narrow-phase.
  [[nodiscard]] bool IfCollides(const std::shared_ptr<Character> &other) const;
  [[nodiscard]] const glm::vec2 GetSize() const
  {
    auto size = this->GetScaledSize();
    return size;
  }

  [[nodiscard]] bool IsHovering(const glm::vec2 &mousePos) const
  {
    auto thisPos = GetPosition();
    auto thisSize = GetSize();
    return (mousePos.x >= thisPos.x - thisSize.x / 2 &&
            mousePos.x <= thisPos.x + thisSize.x / 2 &&
            mousePos.y >= thisPos.y - thisSize.y / 2 &&
            mousePos.y <= thisPos.y + thisSize.y / 2);
  }

  [[nodiscard]] bool IfClicked(const glm::vec2 &mousePos) const
  {
    return IsHovering(mousePos) &&
           Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB);
  }

  void UpdateHoverScale(const glm::vec2 &mousePos, float scaleAmount = 1.2f)
  {
    if (IsHovering(mousePos))
    {
      m_Transform.scale = {scaleAmount, scaleAmount};
    }
    else
    {
      m_Transform.scale = {1.0f, 1.0f};
    }
  }
  // TODO: Add and implement more methods and properties as needed to finish
  // Giraffe Adventure.

private:
  void ResetPosition() { m_Transform.translation = {0, 0}; }

  std::string m_ImagePath;
  PhysicsState m_PhysicsState;
  EntityKind m_EntityKind = EntityKind::Unknown;
  MaterialType m_MaterialType = MaterialType::None;
};

#endif // CHARACTER_HPP
