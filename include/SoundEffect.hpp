#ifndef SFX_HPP
#define SFX_HPP

#include "Util/GameObject.hpp"
#include "Util/SFX.hpp"

class SoundEffect : public Util::GameObject {
public:
  SoundEffect();
  void Play_SFX();

private:
  Util::SFX m_SFX;
};

#endif
