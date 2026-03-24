#ifndef SCENE_HPP
#define SCENE_HPP

#include "BGM.hpp"
// #include "Button.hpp"
// #include "Character.hpp"
// #include "DynamicBackground.hpp"
#include "Util/GameObject.hpp"
// #include "Util/Image.hpp"
#include <functional>

// #include "Util/Transform.hpp"
#include <memory>
#include <vector>

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
  void SetVisible(bool visible) { m_Background->SetVisible(visible); }
  void SetZIndex(float index) { m_Background->SetZIndex(index); }
  void SetBGM(std::shared_ptr<BackgroundMusic> bgm) { m_BGM = bgm; }
  void SetOnUpdate(std::function<void()> onUpdate) { m_OnUpdate = onUpdate; }
  void AddElements(std::shared_ptr<Util::GameObject> element)
  {
    m_Elements.push_back(element);
    AddChild(element);
  }

private:
  std::function<void()> m_OnUpdate = nullptr;
  std::shared_ptr<BackgroundMusic> m_BGM;
  std::shared_ptr<Util::GameObject> m_Background;
  std::vector<std::shared_ptr<Util::GameObject>> m_Elements;
};

#endif // SCENE_HPP
