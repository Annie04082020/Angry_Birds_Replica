#ifndef SCENE_HPP
#define SCENE_HPP

#include "BGM.hpp"
#include "Util/GameObject.hpp"
#include "scenes/ContactManifold.hpp"
#include <functional>
#include <memory>
#include <vector>

class Character; // forward declaration to avoid include dependency

class Scene : public Util::GameObject
{
public:
  // 當 Scene 被建立時，自動把背景圖加到自己的肚子裡 (變成自己的子物件)
  Scene(std::shared_ptr<Util::GameObject> background)
      : m_Background(background)
  {
    if (m_Background)
    {
      AddChild(m_Background);
      m_Background->SetZIndex(-100.0f);
    }
  }
  ~Scene() = default;

  void Init();
  void Update();
  void AddDebugEntity(const std::shared_ptr<Util::GameObject> &obj, float ttl);
  void SetDebugRenderEnabled(bool enabled) { m_DebugRenderEnabled = enabled; }
  [[nodiscard]] bool IsDebugRenderEnabled() const { return m_DebugRenderEnabled; }
  void SetPhysicsPaused(bool paused);
  [[nodiscard]] bool IsPhysicsPaused() const { return m_PhysicsPaused; }
  void RequestPhysicsSingleStep() { m_PhysicsSingleStepRequested = true; }
  // Stabilize environment objects on load by running short physics steps
  void StabilizeEnvironment(int steps = 30);
  void SetVisible(bool visible)
  {
    if (m_Background)
    {
      m_Background->SetVisible(visible);
    }
  }
  void SetZIndex(float index)
  {
    if (m_Background)
    {
      m_Background->SetZIndex(index);
    }
  }
  void SetBGM(std::shared_ptr<BackgroundMusic> bgm) { m_BGM = bgm; }
  std::shared_ptr<BackgroundMusic> GetBGM() const { return m_BGM; }
  void StopBGM()
  {
    if (m_BGM)
    {
      m_BGM->Stop_BGM();
    }
  }
  void SetOnUpdate(std::function<void()> onUpdate) { m_OnUpdate = onUpdate; }
  void AddElements(std::shared_ptr<Util::GameObject> element)
  {
    if (!element)
    {
      return;
    }
    if (m_ElementsTraversalLock > 0)
    {
      m_PendingAddElements.push_back(element);
    }
    else
    {
      m_Elements.push_back(element);
      AddChild(element);
    }
  }

  // Element traversal lock management
  void ReleaseTraversalLock();
  void FlushPendingElements();
  int m_ElementsTraversalLock = 0;

  // Set a Character to be controlled by keyboard for testing.
  void SetControlledCharacter(const std::shared_ptr<Character> &ch) { m_Controlled = ch; }
  const std::shared_ptr<Character> &GetControlledCharacter() const { return m_Controlled; }

  // Score system
  [[nodiscard]] int GetScore() const { return m_Score; }
  void AddScore(int points) { m_Score += points; }
  void SetScore(int score) { m_Score = score; }

  // Called when a character dies (can be overridden by subclasses)
  virtual void OnCharacterDeath(const std::shared_ptr<Character> &) {}

protected:
  // Runs a generic collision detection pass for children of this Scene.
  // Scenes may override `HandleCollision` to react to collisions. The
  // collision now provides SAT/MTV contact data (normal, depth and
  // an approximate contact point) to allow accurate positional correction
  // and torque computation.
  // Rebuild contact manifold list and run the SI solver.
  // `stabilizing` selects stabilization tuning constants while still running
  // warm-start, velocity solve, and position correction.
  void RunCollisionDetection(int passes = 1, bool stabilizing = false);

  // Physics fixed-timestep accumulator (seconds)
  float m_PhysicsStep = 1.0f / 60.0f;
  float m_Accumulator = 0.0f;
  float m_MaxFrameTime = 0.25f; // clamp huge deltas
  int m_MaxSubSteps = 5;
  struct DebugEntity
  {
    std::shared_ptr<Util::GameObject> obj;
    float ttl;
  };
  std::vector<DebugEntity> m_DebugEntities;
  float m_DebugDrawCooldown = 0.0f;
  float m_DebugDrawInterval = 0.08f;
  bool m_DebugRenderEnabled = false;
  bool m_PhysicsPaused = false;
  bool m_PhysicsSingleStepRequested = false;

  void StepPhysics(float dt);
  void DrawPhysicsDebug();
  void AddDebugLine(const glm::vec2 &start,
                    const glm::vec2 &end,
                    const glm::vec4 &color,
                    float thickness,
                    float ttl);

  // World floor Y coordinate. Can be set by caller (e.g. GameScene after loading level)
  float m_WorldFloorY = -294.0f;
  void SetWorldFloorY(float y) { m_WorldFloorY = y; }
  float GetWorldFloorY() const { return m_WorldFloorY; }

  // Grace period: no damage is applied for this many seconds after level load.
  // This prevents initial settling impulses (which are NOT covered by !stabilizing)
  // from destroying objects before the first bird is even launched.
  float m_DamageImmunityTimer = 2.0f;
  [[nodiscard]] bool IsDamageImmune() const { return m_DamageImmunityTimer > 0.0f; }

  // Scaling factor for physics constants (like gravity) to maintain consistent feel across resolutions
  float m_PhysicsScale = 1.0f;
  void SetPhysicsScale(float scale) { m_PhysicsScale = scale; }
  float GetPhysicsScale() const { return m_PhysicsScale; }

  // Persisted contact manifolds for warm-starting the SI solver.
  std::vector<ContactManifold> m_Contacts;

private:
  std::function<void()> m_OnUpdate = nullptr;
  std::shared_ptr<BackgroundMusic> m_BGM;
  std::shared_ptr<Util::GameObject> m_Background;
  std::vector<std::shared_ptr<Util::GameObject>> m_Elements;
  std::vector<std::shared_ptr<Util::GameObject>> m_PendingAddElements;
  std::shared_ptr<Character> m_Controlled = nullptr;
  int m_Score = 0;
};
#endif // SCENE_HPP
