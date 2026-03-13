#include "DynamicBackground.hpp"
#include "Util/Time.hpp"
#include "config.hpp"
#include "Resource.hpp"

DynamicBackground::DynamicBackground() {
  // 創建兩個背景實例，使用相同的圖片
  m_BG1 = std::make_shared<BackgroundImage>(Resource::MOVING_BG_IMAGE);
  m_BG2 = std::make_shared<BackgroundImage>(Resource::MOVING_BG_IMAGE);

  // 初始佈局：背景1在原點，背景2緊接在右側（螢幕寬度處）
  // 這樣兩張圖就拼接成了一個兩倍寬度的長條
  m_BG1->SetPosition({0, 0});
  m_BG2->SetPosition({static_cast<float>(WINDOW_WIDTH), 0});

  // 將背景加入為子物件，這樣它們會隨容器一起移動（如果容器動的話）
  AddChild(m_BG1);
  AddChild(m_BG2);

  // 設定滾動速度
  m_Speed = 200.0f;
}

void DynamicBackground::Update() {
  float dt = Util::Time::GetDeltaTime();
  float movement = m_Speed * dt;

  // 獲取當前兩張圖的位置
  auto pos1 = m_BG1->GetPosition();
  auto pos2 = m_BG2->GetPosition();

  // 兩張圖同步向左移動
  pos1.x -= movement;
  pos2.x -= movement;

  // --- 無限循環邏輯 (你的想法在這裡實現) ---

  // 如果背景1完全移出螢幕左側
  if (pos1.x <= -static_cast<float>(WINDOW_WIDTH)) {
    // 將它「傳送」到背景2的右邊，重新開始拼接
    pos1.x = pos2.x + static_cast<float>(WINDOW_WIDTH);
  }

  // 如果背景2完全移出螢幕左側
  if (pos2.x <= -static_cast<float>(WINDOW_WIDTH)) {
    // 將它「傳送」到背景1的右邊，重新開始拼接
    pos2.x = pos1.x + static_cast<float>(WINDOW_WIDTH);
  }

  // 更新背景物件的實際座標
  m_BG1->SetPosition(pos1);
  m_BG2->SetPosition(pos2);
}
