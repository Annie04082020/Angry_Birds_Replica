#ifndef LEVEL_SELECT_SCENE_HPP
#define LEVEL_SELECT_SCENE_HPP

#include "Button.hpp"
#include "DynamicBackground.hpp"
#include "Scene.hpp"
#include <functional>
#include <memory>
#include <vector>

class LevelSelectScene : public Scene
{
public:
    static std::shared_ptr<LevelSelectScene> Create();
    void Update() override;
    void SetSceneVisible(bool visible);

    void SetOnLevelSelectCallback(std::function<bool(int)> callback)
    {
        m_onLevelSelect = callback;
    }

    void SetOnBackClickCallback(std::function<void()> callback)
    {
        m_onBackClick = callback;
    }

protected:
    LevelSelectScene(std::shared_ptr<DynamicBackground> bg);

private:
    void BuildLevelSelectUI();

    std::shared_ptr<DynamicBackground> m_movingBg;
    std::shared_ptr<Util::GameObject> m_title;
    std::shared_ptr<Util::GameObject> m_subtitle;
    std::shared_ptr<Button> m_backButton;
    std::shared_ptr<Util::GameObject> m_backLabel;
    std::vector<std::shared_ptr<Button>> m_levelButtons;
    std::vector<std::shared_ptr<Util::GameObject>> m_levelLabels;
    glm::vec2 m_backLabelBaseScale = {1.0f, 1.0f};
    std::vector<glm::vec2> m_levelLabelBaseScales;
    std::function<bool(int)> m_onLevelSelect = nullptr;
    std::function<void()> m_onBackClick = nullptr;
};

#endif
