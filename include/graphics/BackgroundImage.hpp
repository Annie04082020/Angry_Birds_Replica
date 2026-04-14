#ifndef BACKGROUND_IMAGE_HPP
#define BACKGROUND_IMAGE_HPP

#include "Core/Context.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/TransformUtils.hpp"
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

  void Update() override
  {
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

    const glm::vec2 viewportSize = Util::GetViewportSize();
    const float viewportWidth = viewportSize.x * (m_WidthPercent / 100.0f);
    const float viewportHeight = viewportSize.y * (m_HeightPercent / 100.0f);
    m_Transform.scale = {viewportWidth / size.x, viewportHeight / size.y};
  }

  std::string m_ImagePath = Resource::SPLASH_IMAGE;
  float m_WidthPercent = 100.0f;
  float m_HeightPercent = 100.0f;
};

#endif // BACKGROUND_IMAGE_HPP
