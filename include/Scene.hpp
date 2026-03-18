#ifndef SCENE_HPP
#define SCENE_HPP

#include "BGM.hpp"
#include "Button.hpp"
#include "Character.hpp"
#include "DynamicBackground.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
// #include "Util/Renderer.hpp"
#include "Util/Transform.hpp"
#include <memory>
#include <vector>

class Scene {
public:
  Scene(std::shared_ptr<Util::GameObject> background)
      : m_Background(background) {}
  ~Scene() = default;

  void Init();
  void Update();
  void Render();
  void AddGameObject(std::shared_ptr<Util::GameObject> gameObject);

private:
  std::shared_ptr<BackgroundMusic> m_BGM;
  std::shared_ptr<Util::GameObject> m_Background;
  std::vector<std::shared_ptr<Util::GameObject>> m_GameObjects;
};

#endif // SCENE_HPP
