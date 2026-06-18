#include "SoundEffect.hpp"
#include "Resource.hpp"

bool SoundEffect::s_IsMuted = false;

SoundEffect::SoundEffect(const std::string &sfxPath) : m_SFX(sfxPath) {}

void SoundEffect::Play_SFX()
{
    if (s_IsMuted)
    {
        return;
    }
    m_SFX.Play();
}

void SoundEffect::SetMuted(const bool muted)
{
    s_IsMuted = muted;
    if (s_IsMuted)
    {
        Mix_HaltChannel(-1);
    }
}

bool SoundEffect::IsMuted()
{
    return s_IsMuted;
}
