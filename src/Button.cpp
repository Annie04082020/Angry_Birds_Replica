#include "Button.hpp"
#include "Util/Image.hpp"

Button::Button(const std::string &ImagePath) {
  SetImage(ImagePath);

  ResetPosition();
}

void Button::SetImage(const std::string &ImagePath) {
  m_ImagePath = ImagePath;
  m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
}
