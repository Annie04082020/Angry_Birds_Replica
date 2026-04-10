#ifndef LEVEL_PARSER_HPP
#define LEVEL_PARSER_HPP

#include "LevelTypes.hpp"

class LevelParser
{
public:
    static ParsedLevelData Parse(const std::string &jsonStr);
};

#endif // LEVEL_PARSER_HPP