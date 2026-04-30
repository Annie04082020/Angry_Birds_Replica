#include "EntityTemplateDatabase.hpp"

#include <limits>

#include "Resource.hpp"
#include "Util/LoadTextFile.hpp"
#include "EntityTemplateParser.hpp"

namespace
{
    ParsedEntityTemplateData g_templates;
    bool g_isLoaded = false;

    bool StartsWith(const std::string &value, const std::string &prefix)
    {
        return value.rfind(prefix, 0) == 0;
    }

    const EntityTemplateDefinition *FindBestMatch(const std::string &imageId)
    {
        const EntityTemplateDefinition *exactMatch = nullptr;
        const EntityTemplateDefinition *prefixMatch = nullptr;
        size_t bestPrefixLength = 0;

        for (const auto &definition : g_templates.templates)
        {
            if (!definition.id.empty() && definition.id == imageId)
            {
                exactMatch = &definition;
                break;
            }

            if (definition.hasMatchPrefix && StartsWith(imageId, definition.matchPrefix) &&
                definition.matchPrefix.length() > bestPrefixLength)
            {
                prefixMatch = &definition;
                bestPrefixLength = definition.matchPrefix.length();
            }
        }

        return exactMatch != nullptr ? exactMatch : prefixMatch;
    }
} // namespace

namespace EntityTemplateDatabase
{
    bool LoadFromFile(const std::string &path)
    {
        const std::string json = Util::LoadTextFile(path);
        if (json.empty())
        {
            g_templates = {};
            g_isLoaded = false;
            return false;
        }

        g_templates = EntityTemplateParser::Parse(json);
        g_isLoaded = true;
        return !g_templates.templates.empty();
    }

    bool LoadDefault()
    {
        return LoadFromFile(Resource::ENTITY_TEMPLATE_DATA);
    }

    const EntityTemplateDefinition *FindTemplate(const std::string &imageId)
    {
        if (!g_isLoaded)
        {
            LoadDefault();
        }

        return FindBestMatch(imageId);
    }
} // namespace EntityTemplateDatabase