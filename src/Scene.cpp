#include "Scene.hpp"

void Scene::Init() {
  if (m_BGM) {
    m_BGM->Play_BGM();
  }
  for (auto &element : m_Elements) {
    element->Init();
  }
}

void Scene::Update() {
  if (m_OnUpdate) {
    m_OnUpdate();
  }
  for (auto &element : m_Elements) {
    element->Update();
  }
}