#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "config.hpp"
#include <string>

namespace Resource {
// Backgrounds
const std::string SPLASH_IMAGE =
    RESOURCE_DIR "/Image/title/SPLASHES_SHEET_1.png";
const std::string MOVING_BG_IMAGE = RESOURCE_DIR "/temp/background.png";

// Characters
const std::string BIRD_R = RESOURCE_DIR "/Image/birds/red_birds/sprite_003.png";
const std::string BIRD_Y = RESOURCE_DIR "/temp/Y.png";
const std::string BIRD_B = RESOURCE_DIR "/temp/B.png";

// UI
const std::string Play_Button = RESOURCE_DIR "/Image/ui/button/play.png";
const std::string Exit_Button = RESOURCE_DIR "/Image/ui/button/sprite_070.png";
const std::string Setting_Button = RESOURCE_DIR "/Image/ui/button/setting.png";
const std::string Setting_Button_Base = RESOURCE_DIR "/Image/ui/button/sprite_068.png";
const std::string Setting_Button_Overlay = RESOURCE_DIR "/Image/ui/button/sprite_030.png";

// Audio - Music
const std::string TITLE_THEME = RESOURCE_DIR "/Audio/music/title_theme.ogg";

// Audio - SFX
const std::string BIRD_LAUNCH_SFX = RESOURCE_DIR "/Audio/sfx/bird_launch.wav";
const std::string SETTING_SFX = RESOURCE_DIR "/Audio/SFX/menu_confirm.wav";

// Level Data
const std::string LEVEL_1_DATA = RESOURCE_DIR "/levels/level_1.json";
} // namespace Resource

#endif // RESOURCE_HPP
