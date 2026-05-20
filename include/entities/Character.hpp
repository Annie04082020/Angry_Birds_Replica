#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <string>
#include <algorithm>

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
    bool isSleeping = false; // temporary sleep state (not permanently immovable)
  };

  // Damage state levels for sprite/image switching
  enum class DamageState
  {
    Undamaged = 0,
    Light = 1,
    Moderate = 2,
    Heavy = 3,
    Critical = 4
  };

  explicit Character(const std::string &ImagePath);

  Character(const Character &) = delete;

  Character(Character &&) = delete;

  Character &operator=(const Character &) = delete;

  Character &operator=(Character &&) = delete;

  [[nodiscard]] const std::string &GetImagePath() const { return m_ImagePath; }

  [[nodiscard]] const std::string &GetBaseImageId() const { return m_BaseImageId; }

  void SetBaseImageId(const std::string &id) { m_BaseImageId = id; }

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

  [[nodiscard]] bool IsSleeping() const { return m_PhysicsState.isSleeping; }

  void SetSleeping(bool sleeping) { m_PhysicsState.isSleeping = sleeping; }

  [[nodiscard]] bool IsImpactActivated() const { return m_ImpactActivated; }

  void SetImpactActivated(bool impactActivated)
  {
    m_ImpactActivated = impactActivated;
  }

  [[nodiscard]] bool ParticipatesInPhysics() const
  {
    return m_ParticipatesInPhysics;
  }

  void SetParticipatesInPhysics(bool participatesInPhysics)
  {
    m_ParticipatesInPhysics = participatesInPhysics;
  }

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

  // Health and damage system
  [[nodiscard]] float GetHealth() const { return m_Health; }

  [[nodiscard]] float GetMaxHealth() const { return m_MaxHealth; }

  void SetHealth(float health)
  {
    m_Health = health;
    m_MaxHealth = std::max(m_MaxHealth, health);
  }

  void SetMaxHealth(float maxHealth)
  {
    if (maxHealth > 0.0f)
    {
      m_MaxHealth = maxHealth;
      m_Health = std::min(m_Health, m_MaxHealth);
    }
  }

  // Set number of available damage state images for dynamic threshold calculation
  void SetNumDamageStates(int numStates)
  {
    m_NumDamageStates = std::max(1, numStates);
  }

  [[nodiscard]] int GetNumDamageStates() const { return m_NumDamageStates; }

  // Returns true if entity dies (health <= 0), false otherwise
  bool ApplyDamage(float damageAmount)
  {
    if (damageAmount < 0.0f)
      damageAmount = 0.0f;
    m_Health = std::max(0.0f, m_Health - damageAmount);
    return m_Health <= 0.0f;
  }

  // Get current damage state for sprite selection
  // Dynamically calculates damage state based on number of available damage variant images
  [[nodiscard]] DamageState GetDamageState() const
  {
    if (m_MaxHealth <= 0.0f)
      return DamageState::Undamaged;

    const float healthPercent = m_Health / m_MaxHealth;

    // With m_NumDamageStates variants, we have equally spaced thresholds.
    // Example: 4 states -> >75% (0), >50% (1), >25% (2), <=25% (3).
    const int maxStateValue = m_NumDamageStates - 1;
    if (maxStateValue <= 0)
      return DamageState::Undamaged;

    for (int i = 0; i < maxStateValue; ++i)
    {
      const float threshold = 1.0f - (static_cast<float>(i + 1) / static_cast<float>(m_NumDamageStates));
      if (healthPercent > threshold)
      {
        return static_cast<DamageState>(i);
      }
    }

    // If health is at or below the lowest threshold, return highest damage state
    return static_cast<DamageState>(maxStateValue);
  }

  // Store previous damage state for detecting changes
  [[nodiscard]] DamageState GetPreviousDamageState() const { return m_PreviousDamageState; }

  void SetPreviousDamageState(DamageState state) { m_PreviousDamageState = state; }

  [[nodiscard]] bool IsDestroyed() const { return m_IsDestroyed; }

  void SetDestroyed(bool destroyed) { m_IsDestroyed = destroyed; }

  // TODO: Add and implement more methods and properties as needed to finish
  // Giraffe Adventure.

private:
  void ResetPosition() { m_Transform.translation = {0, 0}; }

    std::string m_ImagePath;
    std::string m_BaseImageId; // Original image ID for damage state switching
    PhysicsState m_PhysicsState;
    EntityKind m_EntityKind = EntityKind::Unknown;
    MaterialType m_MaterialType = MaterialType::None;
    float m_Health = 1.0f;
    float m_MaxHealth = 1.0f;
    DamageState m_PreviousDamageState = DamageState::Undamaged;
    bool m_IsDestroyed = false;
    int m_NumDamageStates = 5; // Default to 5 states (undamaged + 4 variants)
    bool m_ImpactActivated = true;
    bool m_ParticipatesInPhysics = true;
};

#endif // CHARACTER_HPP
