#include "Util/SFX.hpp"

SoundEffect::SoundEffect() : m_SFX(RESOURCE_DIR "/Audio/sfx/bird_launch.wav") {}

void SoundEffect::Play_SFX() { m_SFX.Play(); }
