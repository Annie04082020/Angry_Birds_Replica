#include "BGM.hpp"
#include "Resource.hpp"

BackgroundMusic::BackgroundMusic()
    : m_BGM(Resource::TITLE_THEME) {}

void BackgroundMusic::Play_BGM() { m_BGM.Play(-1); }
void BackgroundMusic::Stop_BGM() { m_BGM.Pause(); }
void BackgroundMusic::Resume_BGM() { m_BGM.Resume(); }
