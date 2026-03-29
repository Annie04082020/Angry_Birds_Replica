#include "Util/TransformUtils.hpp"

#include "config.hpp"
#include <glm/gtx/matrix_transform_2d.hpp>

namespace Util {
static float g_CameraZoom = 1.0f;

void SetCameraZoom(float zoom) {
    g_CameraZoom = zoom > 0.1f ? zoom : 0.1f; // Clamp to prevent zoom <= 0
}

float GetCameraZoom() {
    return g_CameraZoom;
}

Core::Matrices ConvertToUniformBufferData(const Util::Transform &transform,
                                          const glm::vec2 &size,
                                          const float zIndex) {
    constexpr glm::mat4 eye(1.F);

    constexpr float nearClip = -100;
    constexpr float farClip = 100;

    auto projection =
        glm::ortho<float>(0.0F, 1.0F, 0.0F, 1.0F, nearClip, farClip);
    // Apply global camera zoom to view matrix
    auto view = glm::scale(eye, {1.F / WINDOW_WIDTH / g_CameraZoom,
                                 1.F / WINDOW_HEIGHT / g_CameraZoom, 1.F}) *
                glm::translate(eye, {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0});

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

} // namespace Util
