#ifndef SCENE_HPP
#define SCENE_HPP

#include "BGM.hpp"
#include "Util/GameObject.hpp"
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
    }
  }
  ~Scene() = default;

  void Init() override;
  void Update() override;
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
    m_Elements.push_back(element);
    AddChild(element);
  }

  // Set a Character to be controlled by keyboard for testing.
  void SetControlledCharacter(const std::shared_ptr<Character> &ch) { m_Controlled = ch; }
  const std::shared_ptr<Character> &GetControlledCharacter() const { return m_Controlled; }

protected:
  // Runs a generic collision detection pass for children of this Scene.
  // Scenes may override `HandleCollision` to react to collisions.
  virtual void HandleCollision(const std::shared_ptr<Util::GameObject> &a,
                               const std::shared_ptr<Util::GameObject> &b);
  void RunCollisionDetection();

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
  float m_DebugDrawInterval = 0.05f;

private:
  std::function<void()> m_OnUpdate = nullptr;
  std::shared_ptr<BackgroundMusic> m_BGM;
  std::shared_ptr<Util::GameObject> m_Background;
  std::vector<std::shared_ptr<Util::GameObject>> m_Elements;
  std::shared_ptr<Character> m_Controlled = nullptr;
};

#endif // SCENE_HPP
