#include "Util/TransformUtils.hpp"

#include "SDL.h"
#include "config.hpp"
#include <glm/gtx/matrix_transform_2d.hpp>

namespace Util {
constexpr float MIN_ZOOM = 0.5f;
constexpr float MAX_ZOOM = 1.0f;

static float g_CameraZoom = 1.0f;
static glm::vec2 g_CameraPosition = {0.0f, 0.0f};

void SetCameraZoom(float zoom) {
    g_CameraZoom =
        zoom < MIN_ZOOM ? MIN_ZOOM : (zoom > MAX_ZOOM ? MAX_ZOOM : zoom);
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
    int windowWidth = static_cast<int>(WINDOW_WIDTH);
    int windowHeight = static_cast<int>(WINDOW_HEIGHT);

    if (SDL_Window *window = SDL_GL_GetCurrentWindow()) {
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);

        int drawableWidth = windowWidth;
        int drawableHeight = windowHeight;
        SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
        if (drawableWidth > 0) {
            windowWidth = drawableWidth;
        }
        if (drawableHeight > 0) {
            windowHeight = drawableHeight;
        }
    }

    return {static_cast<float>(windowWidth), static_cast<float>(windowHeight)};
}

Core::Matrices ConvertToUniformBufferData(const Util::Transform &transform,
                                          const glm::vec2 &size,
                                          const float zIndex) {
    constexpr glm::mat4 eye(1.F);

    constexpr float nearClip = -100;
    constexpr float farClip = 100;

    const glm::vec2 viewportSize = GetViewportSize();
    auto projection =
        glm::ortho<float>(0.0F, 1.0F, 0.0F, 1.0F, nearClip, farClip);
    // Apply global camera zoom and position to view matrix
    auto view =
        glm::scale(eye, {1.F / viewportSize.x / g_CameraZoom,
                         1.F / viewportSize.y / g_CameraZoom, 1.F}) *
        glm::translate(eye, {viewportSize.x / 2 - g_CameraPosition.x,
                             viewportSize.y / 2 - g_CameraPosition.y, 0});

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
