#include "GameScene.hpp"
#include "Character.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

bool GameScene::LoadLevel(const std::string &levelPath)
{
    if (!m_LevelManager || !m_LevelManager->LoadLevel(levelPath))
    {
        return false;
    }

    const auto &objects = m_LevelManager->GetGameObjects();
    for (const auto &obj : objects)
    {
        AddElements(obj);
    }

    if (!objects.empty())
    {
        auto controlled = std::dynamic_pointer_cast<Character>(objects.front());
        if (controlled)
        {
            SetControlledCharacter(controlled);
        }
    }

    return true;
}

void GameScene::Update()
{
    HandleBackgroundDrag();
    Scene::Update();
}

void GameScene::HandleBackgroundDrag()
{
    if (!m_DynamicBackground)
    {
        return;
    }

    const glm::vec2 mousePos = Util::Input::GetCursorPosition();
    const bool mouseDown = Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB);

    if (mouseDown)
    {
        if (!m_IsDraggingBackground)
        {
            m_IsDraggingBackground = true;
            m_LastMousePos = mousePos;
            return;
        }

        const glm::vec2 delta = mousePos - m_LastMousePos;
        auto bgPos = m_DynamicBackground->GetPosition();
        m_DynamicBackground->SetPosition(bgPos + delta);
        m_LastMousePos = mousePos;
    }
    else
    {
        m_IsDraggingBackground = false;
    }
}
