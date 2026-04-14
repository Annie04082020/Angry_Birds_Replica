#ifndef LEVEL_MANAGER_HPP
#define LEVEL_MANAGER_HPP

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

class Character; // Forward declaration

/**
 * @class LevelManager
 * @brief Manages loading and storing level data
 *        Handles level JSON file parsing and character creation
 */
class LevelManager
{
public:
    /**
     * @brief Default constructor
     */
    LevelManager() = default;

    /**
     * @brief Load a level from a JSON file
     * @param levelPath Path to the level JSON file
     * @return true if loading was successful, false otherwise
     */
    bool LoadLevel(const std::string &levelPath);

    /**
     * @brief Get all game objects (characters) in the current level
     * @return Reference to the vector of game characters
     */
    const std::vector<std::shared_ptr<Character>> &GetGameObjects() const
    {
        return m_gameObjects;
    }

    /**
     * @brief Get the number of birds in the current level
     * @return Number of available birds
     */
    int GetBirdCount() const
    {
        return m_birdCount;
    }

    /**
     * @brief Get the level name
     * @return Name of the current level
     */
    const std::string &GetLevelName() const
    {
        return m_levelName;
    }

    /**
     * @brief Get background image path for the level
     * @return Path to the background image
     */
    const std::string &GetBackgroundImage() const
    {
        return m_backgroundImage;
    }

    /**
     * @brief Clear all loaded entities
     */
    void Clear()
    {
        m_gameObjects.clear();
        m_resourceMap.clear();
        m_birdCount = 0;
        m_levelName = "";
        m_backgroundImage = "";
    }

private:
    /**
     * @brief Parse JSON object and extract entity data
     * @param jsonStr JSON string to parse
     * @return true if parsing was successful
     */
    bool ParseLevelJson(const std::string &jsonStr);

    std::vector<std::shared_ptr<Character>> m_gameObjects;      ///< All game objects in the current level
    std::unordered_map<std::string, std::string> m_resourceMap; ///< Local resource map (resourceId -> path)
    int m_birdCount = 0;                                        ///< Number of birds available
    std::string m_levelName = "";                               ///< Name of the current level
    std::string m_backgroundImage = "";                         ///< Background image path
};

#endif // LEVEL_MANAGER_HPP
