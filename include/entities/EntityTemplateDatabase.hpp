#ifndef ENTITY_TEMPLATE_DATABASE_HPP
#define ENTITY_TEMPLATE_DATABASE_HPP

#include <string>

#include "EntityTemplateTypes.hpp"

namespace EntityTemplateDatabase
{
    bool LoadFromFile(const std::string &path);

    bool LoadDefault();

    const EntityTemplateDefinition *FindTemplate(const std::string &imageId);
}

#endif // ENTITY_TEMPLATE_DATABASE_HPP