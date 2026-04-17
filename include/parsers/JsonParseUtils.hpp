#ifndef JSON_PARSE_UTILS_HPP
#define JSON_PARSE_UTILS_HPP

#include <string>
#include <vector>
#include <unordered_map>

namespace JsonParseUtils
{
    std::string ExtractString(const std::string &json, const std::string &key,
                              const std::string &fallback = "");
    int ExtractInt(const std::string &json, const std::string &key,
                   int fallback = 0);
    float ExtractFloat(const std::string &json, const std::string &key,
                       float fallback = 0.0f);
    bool ExtractBool(const std::string &json, const std::string &key,
                     bool fallback = false);
    bool HasKey(const std::string &json, const std::string &key);

    // Extract nested object content by key, without outer braces.
    bool ExtractObjectContent(const std::string &json, const std::string &objectKey,
                              std::string &outContent);

    bool ExtractArrayContent(const std::string &json, const std::string &arrayKey,
                             std::string &outContent);

    std::vector<std::string> ExtractObjectContentsFromArray(const std::string &jsonArrayWithBrackets);

    std::unordered_map<std::string, std::string> ExtractStringMapFromObject(
        const std::string &jsonObjectWithBraces);
} // namespace JsonParseUtils

#endif // JSON_PARSE_UTILS_HPP
