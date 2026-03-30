#include "Scene.hpp"
// Collision detection and input
#include "Character.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

void Scene::Init()
{
  if (m_BGM)
  {
    m_BGM->Play_BGM();
  }
  if (m_Background)
  {
    m_Background->Init();
  }
  for (auto &element : m_Elements)
  {
    element->Init();
  }
}

void Scene::Update()
{
  if (m_OnUpdate)
  {
    m_OnUpdate();
  }
  if (m_Background)
  {
    m_Background->Update();
  }
  for (auto &element : m_Elements)
  {
    element->Update();
  }
  // Keyboard control for testing: move controlled character with arrow keys/WASD
  if (m_Controlled)
  {
    float speed = 5.0f; // units per update
    glm::vec2 pos = m_Controlled->GetPosition();
    if (Util::Input::IsKeyDown(Util::Keycode::LEFT) ||
        Util::Input::IsKeyDown(Util::Keycode::A))
    {
      pos.x -= speed;
    }
    if (Util::Input::IsKeyDown(Util::Keycode::RIGHT) ||
        Util::Input::IsKeyDown(Util::Keycode::D))
    {
      pos.x += speed;
    }
    if (Util::Input::IsKeyDown(Util::Keycode::UP) ||
        Util::Input::IsKeyDown(Util::Keycode::W))
    {
      pos.y -= speed;
    }
    if (Util::Input::IsKeyDown(Util::Keycode::DOWN) ||
        Util::Input::IsKeyDown(Util::Keycode::S))
    {
      pos.y += speed;
    }
    m_Controlled->SetPosition(pos);
  }

  // Run collision detection for scene children
  RunCollisionDetection();
}

void Scene::RunCollisionDetection()
{
  auto children = GetChildren();
  for (size_t i = 0; i < children.size(); ++i)
  {
    for (size_t j = i + 1; j < children.size(); ++j)
    {
      auto a = children[i];
      auto b = children[j];

      auto ca = std::dynamic_pointer_cast<Character>(a);
      auto cb = std::dynamic_pointer_cast<Character>(b);

      if (ca && cb)
      {
        if (ca->IfCollides(cb))
        {
          HandleCollision(ca, cb);
        }
      }
    }
  }
}