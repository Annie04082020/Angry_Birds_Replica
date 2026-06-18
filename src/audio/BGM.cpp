#include "BGM.hpp"
#include "Resource.hpp"
#include "SoundEffect.hpp"

BackgroundMusic::BackgroundMusic(const std::string &bgmPath) : m_BGM(bgmPath) {}

void BackgroundMusic::Play_BGM()
{
    if (SoundEffect::IsMuted())
    {
        return;
    }
    m_BGM.Play(-1);
}

void BackgroundMusic::Stop_BGM() { m_BGM.Pause(); }

void BackgroundMusic::Resume_BGM()
{
    if (SoundEffect::IsMuted())
    {
        return;
    }
    m_BGM.Resume();
}
