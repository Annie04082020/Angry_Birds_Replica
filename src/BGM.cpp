#include "BGM.hpp"

BackgroundMusic::BackgroundMusic()
    : m_BGM(RESOURCE_DIR "/Audio/music/title_theme.ogg") {}

void BackgroundMusic::Play_BGM() { m_BGM.Play(-1); }
void BackgroundMusic::Stop_BGM() { m_BGM.Pause(); }
void BackgroundMusic::Resume_BGM() { m_BGM.Resume(); }
