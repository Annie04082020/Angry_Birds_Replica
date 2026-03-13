#ifndef BACKGROUND_IMAGE_HPP
#define BACKGROUND_IMAGE_HPP

#include "Core/Context.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "config.hpp"
#include "Resource.hpp"
#include <string>

class BackgroundImage : public Util::GameObject {

public:
  BackgroundImage()
      : BackgroundImage(Resource::SPLASH_IMAGE) {}

  BackgroundImage(const std::string &path)
      : GameObject(std::make_unique<Util::Image>(path), -10) {
    auto size = m_Drawable->GetSize();
    float scale_x = static_cast<float>(WINDOW_WIDTH) / size.x;
    float scale_y = static_cast<float>(WINDOW_HEIGHT) / size.y;
    m_Transform.scale = {scale_x, scale_y};
    m_ImagePath = path;
  }

  void SetImage(const std::string &path) {
    auto temp = std::dynamic_pointer_cast<Util::Image>(m_Drawable);
    temp->SetImage(path);
  }

  void SetPosition(const glm::vec2 &position) { m_Transform.translation = position; }

  glm::vec2 GetPosition() const { return m_Transform.translation; }

private:
  std::string m_ImagePath = Resource::SPLASH_IMAGE;
};

#endif // BACKGROUND_IMAGE_HPP
