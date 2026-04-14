#ifndef LEVEL_OBJECT_FACTORY_HPP
#define LEVEL_OBJECT_FACTORY_HPP

#include <memory>

#include "LevelTypes.hpp"

class Character;

class LevelObjectFactory
{
public:
    static std::shared_ptr<Character> CreateCharacter(const LevelObjectDefinition &objectDefinition,
                                                      const ParsedLevelData &levelData,
                                                      float runtimeScale);
};

#endif // LEVEL_OBJECT_FACTORY_HPP