#ifndef SCENE_HPP
#define SCENE_HPP

#include "BackgroundImage.hpp"
#include "Button.hpp"
#include "DynamicBackground.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"
#include "Util/Scene.hpp"
#include "Util/Transform.hpp"

class Scene : public Util::Scene {
public:
  Scene() = default;
  ~Scene() = default;

  void Init() override;
  void Update() override;
  void Render() override;

private:
};
