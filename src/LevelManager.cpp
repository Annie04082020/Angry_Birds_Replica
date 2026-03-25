#include "LevelManager.hpp"
#include "Character.hpp"
#include "Util/LoadTextFile.hpp"
#include <sstream>
#include <iostream>
#include <cmath>

namespace {
    // Simple JSON parsing helpers
    [[maybe_unused]] std::string TrimWhitespace(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }

    std::string ExtractJsonString(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos) return "";
        
        // Find the colon after the key
        size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos) return "";
        
        // Find the opening quote
        size_t openQuote = json.find('\"', colonPos);
        if (openQuote == std::string::npos) return "";
        
        // Find the closing quote
        size_t closeQuote = json.find('\"', openQuote + 1);
        if (closeQuote == std::string::npos) return "";
        
        return json.substr(openQuote + 1, closeQuote - openQuote - 1);
    }

    int ExtractJsonInt(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos) return 0;
        
        size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos) return 0;
        
        // Extract the number after the colon
        size_t numStart = colonPos + 1;
        while (numStart < json.length() && (json[numStart] == ' ' || json[numStart] == '\t')) {
            numStart++;
        }
        
        size_t numEnd = numStart;
        while (numEnd < json.length() && (std::isdigit(json[numEnd]) || json[numEnd] == '-')) {
            numEnd++;
        }
        
        if (numStart < numEnd) {
            return std::stoi(json.substr(numStart, numEnd - numStart));
        }
        return 0;
    }

    float ExtractJsonFloat(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos) return 0.0f;
        
        size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos) return 0.0f;
        
        // Extract the number after the colon
        size_t numStart = colonPos + 1;
        while (numStart < json.length() && (json[numStart] == ' ' || json[numStart] == '\t')) {
            numStart++;
        }
        
        size_t numEnd = numStart;
        while (numEnd < json.length() && (std::isdigit(json[numEnd]) || json[numEnd] == '.' || json[numEnd] == '-')) {
            numEnd++;
        }
        
        if (numStart < numEnd) {
            return std::stof(json.substr(numStart, numEnd - numStart));
        }
        return 0.0f;
    }

    std::string PrepareResourcePath(const std::string& resourcePath) {
        // If path is empty or absolute, return as-is
        if (resourcePath.empty() || resourcePath.find(':') != std::string::npos || resourcePath[0] == '/') {
            return resourcePath;
        }
        
        // This is a relative path, prepend RESOURCE_DIR
        std::string prefix = RESOURCE_DIR;
        return prefix + "/" + resourcePath;
    }

    // Extract key-value pairs from a JSON object string
    std::unordered_map<std::string, std::string> ExtractJsonObject(const std::string& json) {
        std::unordered_map<std::string, std::string> result;
        
        // Find the opening brace
        size_t objectStart = json.find('{');
        if (objectStart == std::string::npos) return result;
        
        size_t objectEnd = json.rfind('}');
        if (objectEnd == std::string::npos || objectEnd <= objectStart) return result;
        
        std::string content = json.substr(objectStart + 1, objectEnd - objectStart - 1);
        
        // Parse key-value pairs
        size_t pos = 0;
        while (pos < content.length()) {
            // Find the key
            size_t keyStart = content.find('\"', pos);
            if (keyStart == std::string::npos) break;
            
            size_t keyEnd = content.find('\"', keyStart + 1);
            if (keyEnd == std::string::npos) break;
            
            std::string key = content.substr(keyStart + 1, keyEnd - keyStart - 1);
            
            // Find the colon
            size_t colonPos = content.find(':', keyEnd);
            if (colonPos == std::string::npos) break;
            
            // Find the value
            size_t valueStart = content.find('\"', colonPos);
            if (valueStart == std::string::npos) break;
            
            size_t valueEnd = content.find('\"', valueStart + 1);
            if (valueEnd == std::string::npos) break;
            
            std::string value = content.substr(valueStart + 1, valueEnd - valueStart - 1);
            result[key] = value;
            
            pos = valueEnd + 1;
        }
        
        return result;
    }
}

bool LevelManager::LoadLevel(const std::string& levelPath) {
    std::string fileContent = Util::LoadTextFile(levelPath);
    if (fileContent.empty()) {
        std::cerr << "Failed to load level file: " << levelPath << std::endl;
        return false;
    }
    
    Clear();
    return ParseLevelJson(fileContent);
}

std::unordered_map<std::string, std::string> LevelManager::ExtractResourceMap(const std::string& jsonStr) {
    std::unordered_map<std::string, std::string> resourceMap;
    
    // Find the resources object
    size_t resourcesStart = jsonStr.find("\"resources\"");
    if (resourcesStart == std::string::npos) {
        std::cerr << "No resources object found in level JSON" << std::endl;
        return resourceMap;  // Return empty map if no resources found
    }
    
    // Find the opening brace after "resources"
    size_t objectStart = jsonStr.find('{', resourcesStart);
    if (objectStart == std::string::npos) return resourceMap;
    
    // Find the closing brace for resources object
    size_t braceCount = 0;
    size_t currentPos = objectStart;
    size_t objectEnd = objectStart;
    
    while (currentPos < jsonStr.length()) {
        if (jsonStr[currentPos] == '{') {
            braceCount++;
        } else if (jsonStr[currentPos] == '}') {
            braceCount--;
            if (braceCount == 0) {
                objectEnd = currentPos;
                break;
            }
        }
        currentPos++;
    }
    
    // Extract the resources content
    std::string resourcesContent = jsonStr.substr(objectStart, objectEnd - objectStart + 1);
    
    // Parse key-value pairs from the resources object
    resourceMap = ExtractJsonObject(resourcesContent);
    
    std::cout << "Loaded " << resourceMap.size() << " resource definitions" << std::endl;
    return resourceMap;
}

bool LevelManager::ParseLevelJson(const std::string& jsonStr) {
    // Extract level metadata
    m_levelName = ExtractJsonString(jsonStr, "name");
    m_backgroundImage = ExtractJsonString(jsonStr, "background");
    m_birdCount = ExtractJsonInt(jsonStr, "birds");
    
    // Extract the local resource map first
    m_resourceMap = ExtractResourceMap(jsonStr);
    
    // Find the objects array (changed from "entities")
    size_t objectsStart = jsonStr.find("\"objects\"");
    if (objectsStart == std::string::npos) {
        std::cerr << "No objects array found in level JSON" << std::endl;
        return false;
    }
    
    // Find the array opening bracket [
    size_t arrayStart = jsonStr.find('[', objectsStart);
    size_t arrayEnd = jsonStr.find(']', arrayStart);
    
    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) {
        std::cerr << "Invalid objects array format" << std::endl;
        return false;
    }
    
    std::string objectsContent = jsonStr.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
    
    // Parse each object (separated by commas)
    size_t objectStart = 0;
    int braceCount = 0;
    size_t currentPos = 0;
    
    while (currentPos < objectsContent.length()) {
        if (objectsContent[currentPos] == '{') {
            if (braceCount == 0) {
                objectStart = currentPos;
            }
            braceCount++;
        } else if (objectsContent[currentPos] == '}') {
            braceCount--;
            if (braceCount == 0) {
                std::string entityJson = objectsContent.substr(objectStart + 1, currentPos - objectStart - 1);
                
                // Parse object properties
                std::string typeStr = ExtractJsonString(entityJson, "type");
                float posX = ExtractJsonFloat(entityJson, "x");
                float posY = ExtractJsonFloat(entityJson, "y");
                float scaleX = ExtractJsonFloat(entityJson, "scaleX");
                float scaleY = ExtractJsonFloat(entityJson, "scaleY");
                float rotation = ExtractJsonFloat(entityJson, "rotation");
                
                // Get the resource ID from the object (changed from "resourceId" to "imageId")
                std::string imageId = ExtractJsonString(entityJson, "imageId");
                
                // Look up the actual resource path from the resource map
                std::string resourcePath = "";
                auto it = m_resourceMap.find(imageId);
                if (it != m_resourceMap.end()) {
                    resourcePath = it->second;
                } else {
                    std::cerr << "Warning: Image ID '" << imageId << "' not found in resource map" << std::endl;
                    continue;  // Skip this object if resource not found
                }
                
                // Validate scale (default to 1.0 if not set)
                if (scaleX <= 0.0f) scaleX = 1.0f;
                if (scaleY <= 0.0f) scaleY = 1.0f;
                
                // Prepare resource path
                std::string fullPath = PrepareResourcePath(resourcePath);
                
                // Create character directly
                auto character = std::make_shared<Character>(fullPath);
                character->SetPosition(glm::vec2(posX, posY));
                character->SetScale(glm::vec2(scaleX, scaleY));
                character->SetRotation(rotation);
                character->SetVisible(true);
                
                m_gameObjects.push_back(character);
            }
        }
        currentPos++;
    }
    
    std::cout << "Loaded level: " << m_levelName << std::endl;
    std::cout << "Game objects loaded: " << m_gameObjects.size() << std::endl;
    std::cout << "Available birds: " << m_birdCount << std::endl;
    
    return true;
}
