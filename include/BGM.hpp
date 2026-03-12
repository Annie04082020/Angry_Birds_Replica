#ifndef BGM_HPP
#define BGM_HPP

#include "Util/BGM.hpp"
#include "Util/GameObject.hpp"

class BackgroundMusic : public Util::GameObject {
public:
  BackgroundMusic();
  void Play_BGM();

private:
  Util::BGM m_BGM;
};

#endif
