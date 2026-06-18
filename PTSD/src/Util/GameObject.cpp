#include "Util/GameObject.hpp"
#include "Util/Transform.hpp"
#include "Util/TransformUtils.hpp"

namespace Util {

void GameObject::Draw() {
    if (!m_Visible || m_Drawable == nullptr) {
        return;
    }

    auto data = Util::ConvertToUniformBufferData(
        m_Transform, m_Drawable->GetSize(), m_ZIndex);
    // Adjust pivot translation to account for non-uniform scaling: use the
    // drawable size after applying the object's scale so the pivot maps
    // correctly in model space.
    const glm::vec2 scaledSize = m_Drawable->GetSize() * m_Transform.scale;
    data.m_Model = glm::translate(data.m_Model,
                                  glm::vec3{m_Pivot / scaledSize, 0} * -1.0F);

    m_Drawable->Draw(data);
}

} // namespace Util