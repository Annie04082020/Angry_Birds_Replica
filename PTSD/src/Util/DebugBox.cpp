#include "Util/DebugBox.hpp"

#include "pch.hpp"

#include "Core/Texture.hpp"
#include "Util/TransformUtils.hpp"

namespace Util {

std::unique_ptr<Core::Program> DebugBox::s_Program = nullptr;
std::unique_ptr<Core::VertexArray> DebugBox::s_VertexArray = nullptr;

DebugBox::DebugBox(const glm::vec4 &color, float thickness)
    : m_Color(color),
      m_Thickness(thickness) {
    if (s_Program == nullptr) {
        InitProgram();
    }
    if (s_VertexArray == nullptr) {
        InitVertexArray();
    }

    m_UniformBuffer = std::make_unique<Core::UniformBuffer<Core::Matrices>>(
        *s_Program, "Matrices", 0);
}

void DebugBox::Draw(const Core::Matrices &data) {
    s_Program->Bind();
    s_Program->Validate();
    m_UniformBuffer->SetData(0, data);
    GLint loc = glGetUniformLocation(s_Program->GetId(), "uColor");
    if (loc >= 0) {
        glUniform4f(loc, m_Color.r, m_Color.g, m_Color.b, m_Color.a);
    }
    loc = glGetUniformLocation(s_Program->GetId(), "uThickness");
    if (loc >= 0) {
        glUniform1f(loc, m_Thickness);
    }

    s_VertexArray->Bind();
    s_VertexArray->DrawTriangles();
}

void DebugBox::InitProgram() {
    s_Program = std::make_unique<Core::Program>(
        PTSD_ASSETS_DIR "/shaders/DebugBox.vert",
        PTSD_ASSETS_DIR "/shaders/DebugBox.frag");
    s_Program->Bind();
}

void DebugBox::InitVertexArray() {
    s_VertexArray = std::make_unique<Core::VertexArray>();

    s_VertexArray->AddVertexBuffer(std::make_unique<Core::VertexBuffer>(
        std::vector<float>{
            -0.5F,
            0.5F, // v0
            -0.5F,
            -0.5F, // v1
            0.5F,
            -0.5F, // v2
            0.5F,
            0.5F, // v3
        },
        2));

    s_VertexArray->AddVertexBuffer(std::make_unique<Core::VertexBuffer>(
        std::vector<float>{
            0.0F,
            0.0F, // uv0
            0.0F,
            1.0F, // uv1
            1.0F,
            1.0F, // uv2
            1.0F,
            0.0F, // uv3
        },
        2));

    s_VertexArray->SetIndexBuffer(std::make_unique<Core::IndexBuffer>(
        std::vector<unsigned int>{0, 1, 2, 0, 2, 3}));
}

} // namespace Util
