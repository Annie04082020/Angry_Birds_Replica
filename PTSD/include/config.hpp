#ifndef CONFIG_H
#define CONFIG_H

#include "pch.hpp" // IWYU pragma: export

#include "Util/Logger.hpp"

constexpr const char *TITLE = "Practical Tools for Simple Design";

constexpr int WINDOW_POS_X = SDL_WINDOWPOS_UNDEFINED;
constexpr int WINDOW_POS_Y = SDL_WINDOWPOS_UNDEFINED;

constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int WINDOW_HEIGHT = 720;

// Game viewport dimensions - logical coordinate space for game objects
// This is independent of the physical window size
constexpr float GAME_VIEWPORT_WIDTH = 1920.0f;
constexpr float GAME_VIEWPORT_HEIGHT = 1080.0f;

constexpr Util::Logger::Level DEFAULT_LOG_LEVEL = Util::Logger::Level::DEBUG;

/**
 * @brief FPS limit
 *
 * Set value to 0 to turn off FPS cap
 */
constexpr unsigned int FPS_CAP = 60;

#endif
