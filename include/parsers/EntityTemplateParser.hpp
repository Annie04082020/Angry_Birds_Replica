#ifndef ENTITY_TEMPLATE_PARSER_HPP
#define ENTITY_TEMPLATE_PARSER_HPP

#include <string>

#include "entities/EntityTemplateTypes.hpp"

namespace EntityTemplateParser
{
    ParsedEntityTemplateData Parse(const std::string &jsonStr);
}

#endif // ENTITY_TEMPLATE_PARSER_HPP