#include "Util/TransformUtils.hpp"

#include "Core/Context.hpp"
#include "config.hpp"
#include <algorithm>
#include <glm/gtx/matrix_transform_2d.hpp>

namespace Util {
namespace {
glm::vec2 g_CameraPosition{0.0f, 0.0f};
float g_CameraZoom = 1.0f;
} // namespace

Core::Matrices ConvertToUniformBufferData(const Util::Transform &transform,
                                          const glm::vec2 &size,
                                          const float zIndex) {
    constexpr glm::mat4 eye(1.F);

    constexpr float nearClip = -100;
    constexpr float farClip = 100;

    auto projection =
        glm::ortho<float>(0.0F, 1.0F, 0.0F, 1.0F, nearClip, farClip);
    auto view =
        glm::scale(eye, {1.F / WINDOW_WIDTH, 1.F / WINDOW_HEIGHT, 1.F}) *
        glm::translate(eye, {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0}) *
        glm::scale(eye, {g_CameraZoom, g_CameraZoom, 1.F}) *
        glm::translate(eye, {-g_CameraPosition.x, -g_CameraPosition.y, 0.0f});

    // TODO: TRS comment
    auto model = glm::translate(eye, {transform.translation, zIndex}) *
                 glm::rotate(eye, transform.rotation, glm::vec3(0, 0, 1)) *
                 glm::scale(eye, {transform.scale * size, 1});

    Core::Matrices data = {
        model,
        projection * view,
    };

    return data;
}

void SetCameraZoom(float zoom) {
    g_CameraZoom = std::clamp(zoom, 0.1f, 4.0f);
}

float GetCameraZoom() {
    return g_CameraZoom;
}

void SetCameraPosition(const glm::vec2 &position) {
    g_CameraPosition = position;
}

glm::vec2 GetCameraPosition() {
    return g_CameraPosition;
}

glm::vec2 GetViewportSize() {
    const auto context = Core::Context::GetInstance();
    if (!context) {
        return {0.0f, 0.0f};
    }

    return {
        static_cast<float>(context->GetWindowWidth()),
        static_cast<float>(context->GetWindowHeight()),
    };
}

} // namespace Util
