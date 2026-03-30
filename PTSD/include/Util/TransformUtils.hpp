#ifndef UTIL_TRANSFORM_UTILS_HPP
#define UTIL_TRANSFORM_UTILS_HPP

#include "Core/Drawable.hpp"

#include "Util/Transform.hpp"
#include "pch.hpp"

namespace Util {

/**
 * @brief Set global camera zoom factor (apply to all rendered objects).
 * @param zoom Zoom factor (1.0 = normal, > 1.0 = zoom in, < 1.0 = zoom out)
 */
void SetCameraZoom(float zoom);

/**
 * @brief Get current global camera zoom factor.
 */
float GetCameraZoom();

/**
 * @brief Converts a Transform object into uniform buffer data.
 *
 * Converts transform data in Core::UniformBuffer format.
 * Call it and pass the returned value to Core::UniformBuffer->setData().
 *
 * @param transform The Transform object to be converted.
 * @param size The size of the object.
 * @param zIndex The z-index of the transformation.
 * @return A Matrices object representing the uniform buffer data.
 *
 */
Core::Matrices ConvertToUniformBufferData(const Util::Transform &transform,
                                          const glm::vec2 &size, float zIndex);

} // namespace Util

#endif // UTIL_TRANSFORM_UTILS_HPP
