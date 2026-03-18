#include "Scene.hpp"

void Scene::Init() { m_BGM->Play_BGM(); }

void Scene::Update() {
  // 1. 更新背景 (如果有背景的話)
  if (m_Background) {
    // m_Background->Update();  // 假設你有寫 Update 函式
  }

  // 2. 更新畫面上所有的 GameObjects
  for (auto &obj : m_GameObjects) {
    // obj->Update(); // 呼叫每個物件，例如小鳥往前飛、小豬掉下來
  }
}

void Scene::Render() {
  // 先畫背景，再畫物件（因為最先畫的會被壓在最底層）
  if (m_Background) {
    
    // m_Background->Draw(); // 假設你的背景有 Draw() 或 Render()
  }

  for (auto &obj : m_GameObjects) {
    // obj->Draw();
  }
}

void Scene::AddGameObject(std::shared_ptr<Util::GameObject> gameObject) {
  m_GameObjects.push_back(gameObject);
}