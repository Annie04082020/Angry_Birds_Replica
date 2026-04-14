#include "JsonParseUtils.hpp"

#include <cctype>

namespace JsonParseUtils
{
    std::string ExtractString(const std::string &json, const std::string &key,
                              const std::string &fallback)
    {
        const std::string searchKey = "\"" + key + "\"";
        const size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos)
            return fallback;

        const size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos)
            return fallback;

        const size_t openQuote = json.find('"', colonPos);
        if (openQuote == std::string::npos)
            return fallback;

        const size_t closeQuote = json.find('"', openQuote + 1);
        if (closeQuote == std::string::npos)
            return fallback;

        return json.substr(openQuote + 1, closeQuote - openQuote - 1);
    }

    int ExtractInt(const std::string &json, const std::string &key, int fallback)
    {
        const std::string searchKey = "\"" + key + "\"";
        const size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos)
            return fallback;

        const size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos)
            return fallback;

        size_t numStart = colonPos + 1;
        while (numStart < json.length() &&
               (json[numStart] == ' ' || json[numStart] == '\t'))
        {
            ++numStart;
        }

        size_t numEnd = numStart;
        while (numEnd < json.length() &&
               (std::isdigit(json[numEnd]) || json[numEnd] == '-'))
        {
            ++numEnd;
        }

        if (numStart < numEnd)
        {
            try
            {
                return std::stoi(json.substr(numStart, numEnd - numStart));
            }
            catch (...)
            {
                return fallback;
            }
        }

        return fallback;
    }

    float ExtractFloat(const std::string &json, const std::string &key,
                       float fallback)
    {
        const std::string searchKey = "\"" + key + "\"";
        const size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos)
            return fallback;

        const size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos)
            return fallback;

        size_t numStart = colonPos + 1;
        while (numStart < json.length() &&
               (json[numStart] == ' ' || json[numStart] == '\t' ||
                json[numStart] == '\n' || json[numStart] == '\r'))
        {
            ++numStart;
        }

        size_t numEnd = numStart;
        while (numEnd < json.length() &&
               (std::isdigit(json[numEnd]) || json[numEnd] == '.' ||
                json[numEnd] == '-'))
        {
            ++numEnd;
        }

        if (numStart < numEnd)
        {
            try
            {
                return std::stof(json.substr(numStart, numEnd - numStart));
            }
            catch (...)
            {
                return fallback;
            }
        }

        return fallback;
    }

    bool ExtractBool(const std::string &json, const std::string &key,
                     bool fallback)
    {
        const std::string searchKey = "\"" + key + "\"";
        const size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos)
            return fallback;

        const size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos)
            return fallback;

        size_t valueStart = colonPos + 1;
        while (valueStart < json.length() &&
               (json[valueStart] == ' ' || json[valueStart] == '\t' ||
                json[valueStart] == '\n' || json[valueStart] == '\r'))
        {
            ++valueStart;
        }

        if (valueStart >= json.length())
            return fallback;

        if (json.compare(valueStart, 4, "true") == 0)
            return true;
        if (json.compare(valueStart, 5, "false") == 0)
            return false;

        size_t valueEnd = valueStart;
        while (valueEnd < json.length() &&
               (std::isdigit(static_cast<unsigned char>(json[valueEnd])) ||
                json[valueEnd] == '-' || json[valueEnd] == '.'))
        {
            ++valueEnd;
        }

        if (valueEnd > valueStart)
        {
            try
            {
                return std::stof(json.substr(valueStart, valueEnd - valueStart)) != 0.0f;
            }
            catch (...)
            {
                return fallback;
            }
        }

        return fallback;
    }

    bool HasKey(const std::string &json, const std::string &key)
    {
        return json.find("\"" + key + "\"") != std::string::npos;
    }

    bool ExtractObjectContent(const std::string &json, const std::string &objectKey,
                              std::string &outContent)
    {
        const std::string sectionKey = "\"" + objectKey + "\"";
        const size_t sectionPos = json.find(sectionKey);
        if (sectionPos == std::string::npos)
        {
            return false;
        }

        const size_t openBrace = json.find('{', sectionPos + sectionKey.length());
        if (openBrace == std::string::npos)
        {
            return false;
        }

        int braceCount = 0;
        for (size_t i = openBrace; i < json.length(); ++i)
        {
            if (json[i] == '{')
            {
                ++braceCount;
            }
            else if (json[i] == '}')
            {
                --braceCount;
                if (braceCount == 0)
                {
                    outContent = json.substr(openBrace + 1, i - openBrace - 1);
                    return true;
                }
            }
        }

        return false;
    }

    std::unordered_map<std::string, std::string> ExtractStringMapFromObject(
        const std::string &jsonObjectWithBraces)
    {
        std::unordered_map<std::string, std::string> result;

        const size_t objectStart = jsonObjectWithBraces.find('{');
        if (objectStart == std::string::npos)
            return result;

        const size_t objectEnd = jsonObjectWithBraces.rfind('}');
        if (objectEnd == std::string::npos || objectEnd <= objectStart)
            return result;

        const std::string content =
            jsonObjectWithBraces.substr(objectStart + 1, objectEnd - objectStart - 1);

        size_t pos = 0;
        while (pos < content.length())
        {
            const size_t keyStart = content.find('"', pos);
            if (keyStart == std::string::npos)
                break;

            const size_t keyEnd = content.find('"', keyStart + 1);
            if (keyEnd == std::string::npos)
                break;

            const std::string key = content.substr(keyStart + 1, keyEnd - keyStart - 1);

            const size_t colonPos = content.find(':', keyEnd);
            if (colonPos == std::string::npos)
                break;

            const size_t valueStart = content.find('"', colonPos);
            if (valueStart == std::string::npos)
                break;

            const size_t valueEnd = content.find('"', valueStart + 1);
            if (valueEnd == std::string::npos)
                break;

            result[key] = content.substr(valueStart + 1, valueEnd - valueStart - 1);
            pos = valueEnd + 1;
        }

        return result;
    }
} // namespace JsonParseUtils
