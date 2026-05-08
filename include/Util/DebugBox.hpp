#ifndef UTIL_DEBUG_BOX_HPP
#define UTIL_DEBUG_BOX_HPP

#include "Core/Drawable.hpp"
#include <glm/glm.hpp>

namespace Util {
class DebugBox : public Core::Drawable {
public:
    DebugBox(const glm::vec4 &color, float thickness)
        : m_Color(color), m_Thickness(thickness) {}

    void Draw(const Core::Matrices &data) override {}
    glm::vec2 GetSize() const override { return {0.0f, 0.0f}; }

private:
    glm::vec4 m_Color{1.0f, 0.0f, 0.0f, 1.0f};
    float m_Thickness = 1.0f;
};

} // namespace Util

#endif // UTIL_DEBUG_BOX_HPP
