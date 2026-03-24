#ifndef DYNAMICBACKGROUND_HPP
#define DYNAMICBACKGROUND_HPP

#include "BackgroundImage.hpp"
#include <string>
#include <memory>

class DynamicBackground : public Util::GameObject
{
public:
  DynamicBackground(const std::string &path);

  // 更新背景位置，實現滾動邏輯
  void Update() override;

  // 設定滾動速度
  void SetSpeed(float speed) { m_Speed = speed; }

  // 設定整體背景容器的位置
  void SetPosition(const glm::vec2 &position)
  {
    m_Transform.translation = position;
  }

  // 獲取整體背景容器的位置
  glm::vec2 GetPosition() const { return m_Transform.translation; }

private:
  std::shared_ptr<BackgroundImage> m_BG1; // 第一張背景圖（初始在螢幕內）
  std::shared_ptr<BackgroundImage> m_BG2; // 第二張背景圖（初始在螢幕右側外）
  float m_Speed = 100.0f;                 // 滾動速度（像素/秒）
};

#endif // DYNAMICBACKGROUND_HPP