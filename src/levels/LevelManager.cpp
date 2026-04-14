#include "LevelManager.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "Character.hpp"
#include "LevelObjectFactory.hpp"
#include "LevelParser.hpp"
#include "Util/LoadTextFile.hpp"
#include "Util/TransformUtils.hpp"
#include "config.hpp"

namespace
{
    float GetRuntimeLevelScale()
    {
        const glm::vec2 viewportSize = Util::GetViewportSize();
        const int windowW = static_cast<int>(viewportSize.x);
        const int windowH = static_cast<int>(viewportSize.y);

        if (windowW <= 0 || windowH <= 0)
        {
            std::cerr << "LevelManager: failed to query drawable size, using scale=1.0" << std::endl;
            return 1.0f;
        }

        const float scaleX = static_cast<float>(windowW) / static_cast<float>(WINDOW_WIDTH);
        const float scaleY = static_cast<float>(windowH) / static_cast<float>(WINDOW_HEIGHT);

        float scale = std::min(scaleX, scaleY);
        if (scale <= 0.0f)
        {
            scale = 1.0f;
        }

        std::cout << "LevelManager: runtime window=" << windowW << "x" << windowH
                  << ", levelScale=" << scale << std::endl;
        return scale;
    }
}

bool LevelManager::LoadLevel(const std::string &levelPath)
{
    std::string fileContent = Util::LoadTextFile(levelPath);
    if (fileContent.empty())
    {
        std::cerr << "Failed to load level file: " << levelPath << std::endl;
        return false;
    }

    Clear();
    return ParseLevelJson(fileContent);
}

bool LevelManager::ParseLevelJson(const std::string &jsonStr)
{
    const float runtimeScale = GetRuntimeLevelScale();
    const ParsedLevelData parsedLevelData = LevelParser::Parse(jsonStr);

    m_levelName = parsedLevelData.levelName;
    m_backgroundImage = parsedLevelData.backgroundImage;
    m_birdCount = parsedLevelData.birdCount;
    m_resourceMap = parsedLevelData.resourceMap;

    for (const auto &objectDefinition : parsedLevelData.objects)
    {
        auto character = LevelObjectFactory::CreateCharacter(objectDefinition, parsedLevelData, runtimeScale);
        if (!character)
        {
            continue;
        }

        m_gameObjects.push_back(character);
    }

    std::cout << "Loaded level: " << m_levelName << std::endl;
    std::cout << "Game objects loaded: " << m_gameObjects.size() << std::endl;
    std::cout << "Available birds: " << m_birdCount << std::endl;

    return true;
}
