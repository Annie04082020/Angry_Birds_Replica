#include "BackgroundImage.hpp"

class DynamicBackground : public BackgroundImage {
public:
  DynamicBackground();
  void Update(float dt) override;
  void SetSpeed(float speed) { m_Speed = speed; }

private:
  float m_Speed;
};