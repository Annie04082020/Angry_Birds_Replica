#ifndef BACKGROUND_IMAGE_HPP
#define BACKGROUND_IMAGE_HPP

#include "Core/Context.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "config.hpp"
#include "Resource.hpp"
#include <string>

class BackgroundImage : public Util::GameObject
{

public:
  BackgroundImage()
      : BackgroundImage(Resource::SPLASH_IMAGE) {}

  BackgroundImage(const std::string &path)
      : BackgroundImage(path, 100.0f, 100.0f) {}

  BackgroundImage(const std::string &path, float widthPercent,
                  float heightPercent)
      : GameObject(std::make_unique<Util::Image>(path), -10)
  {
    m_WidthPercent = widthPercent;
    m_HeightPercent = heightPercent;
    ApplyViewportScale();
    m_ImagePath = path;
  }

  void SetImage(const std::string &path)
  {
    auto temp = std::dynamic_pointer_cast<Util::Image>(m_Drawable);
    temp->SetImage(path);
    m_ImagePath = path;
    ApplyViewportScale();
  }

  void SetViewportPercent(float widthPercent, float heightPercent)
  {
    m_WidthPercent = widthPercent;
    m_HeightPercent = heightPercent;
    ApplyViewportScale();
  }

  void SetPosition(const glm::vec2 &position) { m_Transform.translation = position; }

  glm::vec2 GetPosition() const { return m_Transform.translation; }

private:
  void ApplyViewportScale()
  {
    const auto size = m_Drawable->GetSize();
    if (size.x <= 0.0f || size.y <= 0.0f)
    {
      return;
    }

    int windowWidth = static_cast<int>(WINDOW_WIDTH);
    int windowHeight = static_cast<int>(WINDOW_HEIGHT);
    if (SDL_Window *window = SDL_GL_GetCurrentWindow())
    {
      SDL_GetWindowSize(window, &windowWidth, &windowHeight);
      int drawableWidth = windowWidth;
      int drawableHeight = windowHeight;
      SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
      if (drawableWidth > 0)
      {
        windowWidth = drawableWidth;
      }
      if (drawableHeight > 0)
      {
        windowHeight = drawableHeight;
      }
    }

    const float viewportWidth = static_cast<float>(windowWidth) * (m_WidthPercent / 100.0f);
    const float viewportHeight = static_cast<float>(windowHeight) * (m_HeightPercent / 100.0f);
    m_Transform.scale = {viewportWidth / size.x, viewportHeight / size.y};
  }

  std::string m_ImagePath = Resource::SPLASH_IMAGE;
  float m_WidthPercent = 100.0f;
  float m_HeightPercent = 100.0f;
};

#endif // BACKGROUND_IMAGE_HPP
