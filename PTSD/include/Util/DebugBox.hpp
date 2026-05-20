#ifndef UTIL_DEBUG_BOX_HPP
#define UTIL_DEBUG_BOX_HPP

#include "pch.hpp"

#include "Core/Drawable.hpp"
#include "Core/Program.hpp"
#include "Core/UniformBuffer.hpp"
#include "Core/VertexArray.hpp"

#include "Util/Transform.hpp"

namespace Util {
class DebugBox : public Core::Drawable {
public:
    DebugBox(const glm::vec4 &color = glm::vec4(1, 0, 0, 1),
             float thickness = 0.02f);
    ~DebugBox() override = default;

    void Draw(const Core::Matrices &data) override;
    glm::vec2 GetSize() const override { return m_Size; }

private:
    void InitProgram();
    void InitVertexArray();

    static std::unique_ptr<Core::Program> s_Program;
    static std::unique_ptr<Core::VertexArray> s_VertexArray;
    std::unique_ptr<Core::UniformBuffer<Core::Matrices>> m_UniformBuffer;

    glm::vec4 m_Color;
    float m_Thickness;
    glm::vec2 m_Size = {1.0f, 1.0f};
};
} // namespace Util

#endif
