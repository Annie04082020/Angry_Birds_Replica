#include "SoundEffect.hpp"
#include "Resource.hpp"

SoundEffect::SoundEffect(const std::string &sfxPath) : m_SFX(sfxPath) {}

void SoundEffect::Play_SFX() { m_SFX.Play(); }
