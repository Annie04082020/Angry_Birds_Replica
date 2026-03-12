#ifndef BACKGROUND_IMAGE_HPP
#define BACKGROUND_IMAGE_HPP

#include "Core/Context.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "config.hpp"

class BackgroundImage : public Util::GameObject {

public:
  BackgroundImage()
      : GameObject(std::make_unique<Util::Image>(
                       RESOURCE_DIR "/Image/backgrounds/SPLASHES_SHEET_1.png"),
                   -10) {
    auto size = m_Drawable->GetSize();
    float scale_x = static_cast<float>(WINDOW_WIDTH) / size.x;
    float scale_y = static_cast<float>(WINDOW_HEIGHT) / size.y;
    m_Transform.scale = {scale_x, scale_y};
  }

  void NextPhase(const int phase) {
    auto temp = std::dynamic_pointer_cast<Util::Image>(m_Drawable);
    temp->SetImage(ImagePath(phase));
  }

private:
  inline std::string ImagePath(const int phase) {
    // return RESOURCE_DIR"/Image/Background/phase" + std::to_string(phase) +
    // ".png";
    return RESOURCE_DIR "/Image/backgrounds/SPLASHES_SHEET_1.png";
  }
};

#endif // BACKGROUND_IMAGE_HPP
