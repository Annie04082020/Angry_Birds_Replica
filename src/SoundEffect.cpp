#include "SoundEffect.hpp"
#include "Resource.hpp"

SoundEffect::SoundEffect() : m_SFX(Resource::BIRD_LAUNCH_SFX) {}

void SoundEffect::Play_SFX() { m_SFX.Play(); }
