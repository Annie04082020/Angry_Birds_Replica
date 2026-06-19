#include "effects/FloatingScoreManager.hpp"

#include "Util/Text.hpp"
#include "Util/Time.hpp"
#include "config.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>

namespace
{
    constexpr const char *kUIFont = RESOURCE_DIR "/font/angrybirds-regular.ttf";

    std::string FormatScore(const int score)
    {
        std::ostringstream stream;
        stream << score;
        return stream.str();
    }

    class FloatingTextObject : public Util::GameObject
    {
    public:
        FloatingTextObject(const std::shared_ptr<Util::Text> &drawable,
                           const glm::vec2 &position,
                           const glm::vec2 &velocity,
                           const Util::Color &baseColor,
                           const float zIndex,
                           const float lifeTime)
            : Util::GameObject(drawable, zIndex),
              m_DrawableText(drawable),
              m_Velocity(velocity),
              m_BaseColor(baseColor),
              m_LifeTime(lifeTime),
              m_RemainingLife(lifeTime)
        {
            m_Transform.translation = position;
        }

        void Update() override
        {
            if (!m_DrawableText || m_RemainingLife <= 0.0f)
            {
                return;
            }

            const float deltaSec = std::max(0.0f, Util::Time::GetDeltaTimeMs() / 1000.0f);
            m_RemainingLife = std::max(0.0f, m_RemainingLife - deltaSec);
            m_Transform.translation += m_Velocity * deltaSec;
            m_Velocity *= std::pow(0.92f, deltaSec * 60.0f);
            m_Transform.scale = {1.0f, 1.0f};

            Util::Color fadedColor = m_BaseColor;
            fadedColor.a = 255.0f;
            m_DrawableText->SetColor(fadedColor);

            if (m_RemainingLife <= 0.0f)
            {
                SetVisible(false);
            }
        }

    private:
        std::shared_ptr<Util::Text> m_DrawableText = nullptr;
        glm::vec2 m_Velocity{0.0f, 45.0f};
        Util::Color m_BaseColor = Util::Color::FromRGB(255, 255, 255);
        float m_LifeTime = 0.8f;
        float m_RemainingLife = 0.8f;
    };
}

void FloatingScoreManager::SpawnOutlinedText(const AddTimedElementFunction &addTimedElement,
                                             const glm::vec2 &position,
                                             const std::string &text,
                                             const Util::Color &frontColor,
                                             const int fontSize,
                                             const float lifeTime,
                                             const glm::vec2 &velocity) const
{
    const Util::Color outlineColor = Util::Color::FromRGB(120, 70, 45, 255);
    const std::array<glm::vec2, 4> offsets = {
        glm::vec2{-2.5f, 0.0f},
        glm::vec2{2.5f, 0.0f},
        glm::vec2{0.0f, -2.5f},
        glm::vec2{0.0f, 2.5f}};

    for (const auto &offset : offsets)
    {
        auto shadowDrawable = std::make_shared<Util::Text>(kUIFont, fontSize, text, outlineColor);
        auto shadowObject = std::make_shared<FloatingTextObject>(
            shadowDrawable, position + offset, velocity, outlineColor, 92.0f, lifeTime);
        addTimedElement(shadowObject, lifeTime);
    }

    auto frontDrawable = std::make_shared<Util::Text>(kUIFont, fontSize, text, frontColor);
    auto frontObject = std::make_shared<FloatingTextObject>(
        frontDrawable, position, velocity + glm::vec2{0.0f, 6.0f}, frontColor, 93.0f, lifeTime);
    addTimedElement(frontObject, lifeTime);
}

void FloatingScoreManager::SpawnScore(const AddTimedElementFunction &addTimedElement,
                                      const glm::vec2 &position,
                                      const int points,
                                      const Util::Color &frontColor) const
{
    if (points <= 0)
    {
        return;
    }

    const int fontSize = points >= 1000 ? 38 : (points >= 500 ? 34 : 28);
    const float lifeTime = points >= 1000 ? 1.0f : 0.8f;
    const glm::vec2 velocity = points >= 1000 ? glm::vec2{0.0f, 58.0f} : glm::vec2{0.0f, 44.0f};
    SpawnOutlinedText(addTimedElement, position + glm::vec2{0.0f, 8.0f}, FormatScore(points),
                      frontColor, fontSize, lifeTime, velocity);
}
