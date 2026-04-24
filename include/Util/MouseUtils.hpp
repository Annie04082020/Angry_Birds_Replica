// Mouse / hit test helpers (header-only)
#pragma once

#include "Util/Input.hpp"
#include "Util/GameObject.hpp"
#include <glm/vec2.hpp>

namespace Util
{
    namespace MouseUtils
    {
        inline bool IsClickedOver(const glm::vec2 &mousePos, const std::shared_ptr<Util::GameObject> &obj, Util::Keycode button)
        {
            if (!obj)
                return false;
            auto size = obj->GetScaledSize();
            auto pos = obj->m_Transform.translation;
            bool inside = mousePos.x >= pos.x - size.x / 2 && mousePos.x <= pos.x + size.x / 2 &&
                          mousePos.y >= pos.y - size.y / 2 && mousePos.y <= pos.y + size.y / 2;
            return inside && Util::Input::IsKeyDown(button);
        }

    } // namespace MouseUtils
} // namespace Util
