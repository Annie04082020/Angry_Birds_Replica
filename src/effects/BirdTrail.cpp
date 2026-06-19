#include "effects/BirdTrail.hpp"

#include "BirdLaunchController.hpp"
#include "Character.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"
#include "config.hpp"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr const char *kDotImage = RESOURCE_DIR "/Image/assets/sprite_022.png";
    constexpr const char *kHeadImage = RESOURCE_DIR "/Image/assets/sprite_032.png";
    constexpr int kDotPoolSize = 140;
    constexpr float kDotScale = 0.82f;
    constexpr float kDotLifetime = 0.85f;
    constexpr float kEmitDistance = 18.0f;
    constexpr float kMinSpeed = 120.0f;

    class BirdTrailDotObject : public Util::GameObject
    {
    public:
        BirdTrailDotObject(const std::shared_ptr<Util::Image> &drawable,
                           const float zIndex,
                           const float lifeTime)
            : Util::GameObject(drawable, zIndex),
              m_DrawableImage(drawable),
              m_LifeTime(lifeTime)
        {
            SetVisible(false);
        }

        void Activate(const glm::vec2 &position, const glm::vec2 &scale, const float opacity)
        {
            m_RemainingLife = m_LifeTime;
            m_Transform.translation = position;
            m_Transform.scale = scale;
            SetVisible(true);
            if (m_DrawableImage)
            {
                m_DrawableImage->SetOpacity(opacity);
            }
        }

        void Deactivate()
        {
            m_RemainingLife = 0.0f;
            SetVisible(false);
            if (m_DrawableImage)
            {
                m_DrawableImage->SetOpacity(0.0f);
            }
        }

        void Update() override
        {
            if (!m_DrawableImage || m_RemainingLife <= 0.0f)
            {
                if (m_Visible)
                {
                    SetVisible(false);
                }
                return;
            }

            const float deltaSec = std::max(0.0f, Util::Time::GetDeltaTimeMs() / 1000.0f);
            m_RemainingLife = std::max(0.0f, m_RemainingLife - deltaSec);
            m_DrawableImage->SetOpacity(m_LifeTime > 0.0f ? m_RemainingLife / m_LifeTime : 0.0f);
            if (m_RemainingLife <= 0.0f)
            {
                Deactivate();
            }
        }

    private:
        std::shared_ptr<Util::Image> m_DrawableImage = nullptr;
        float m_LifeTime = 0.8f;
        float m_RemainingLife = 0.0f;
    };
}

void BirdTrail::Build(const AddElementFunction &addElement)
{
    if (!m_Dots.empty())
    {
        return;
    }

    const glm::vec2 scale{kDotScale, kDotScale};
    for (int i = 0; i < kDotPoolSize; ++i)
    {
        const char *imagePath = (i % 5 == 0) ? kHeadImage : kDotImage;
        auto drawable = std::make_shared<Util::Image>(imagePath);
        drawable->SetOpacity(0.0f);
        auto dot = std::make_shared<BirdTrailDotObject>(drawable, 94.0f, kDotLifetime);
        dot->m_Transform.scale = scale;
        addElement(dot);
        m_Dots.push_back(dot);
    }
}

void BirdTrail::Reset()
{
    m_LastEmitPositions.clear();
    m_ActiveDotCount = 0;
    m_LastLaunchSequence = 0;
    for (const auto &dot : m_Dots)
    {
        if (auto trailDot = std::dynamic_pointer_cast<BirdTrailDotObject>(dot))
        {
            trailDot->Deactivate();
        }
        else if (dot)
        {
            dot->SetVisible(false);
        }
    }
}

void BirdTrail::Update(const std::shared_ptr<BirdLaunchController> &birdLaunchController)
{
    if (!birdLaunchController || m_Dots.empty())
    {
        return;
    }

    const int launchSequence = birdLaunchController->GetLaunchSequence();
    if (launchSequence != m_LastLaunchSequence)
    {
        m_LastLaunchSequence = launchSequence;
        m_LastEmitPositions.clear();
        m_ActiveDotCount = 0;
        for (const auto &dot : m_Dots)
        {
            if (auto trailDot = std::dynamic_pointer_cast<BirdTrailDotObject>(dot))
            {
                trailDot->Deactivate();
            }
        }
    }

    if (!birdLaunchController->HasBirdInFlight())
    {
        m_LastEmitPositions.clear();
        return;
    }

    const glm::vec2 dotScale{kDotScale, kDotScale};
    const auto &birds = birdLaunchController->GetActiveBirdsInFlight();
    for (const auto &bird : birds)
    {
        if (!bird || bird->IsSleeping() || bird->IsStatic() || glm::length(bird->GetVelocity()) < kMinSpeed)
        {
            continue;
        }

        const glm::vec2 position = bird->GetPosition();
        const Character *key = bird.get();
        const auto it = m_LastEmitPositions.find(key);
        if (it != m_LastEmitPositions.end() && glm::distance(it->second, position) < kEmitDistance)
        {
            continue;
        }

        m_LastEmitPositions[key] = position;

        if (m_ActiveDotCount >= m_Dots.size())
        {
            continue;
        }

        auto dot = std::dynamic_pointer_cast<BirdTrailDotObject>(m_Dots[m_ActiveDotCount]);
        ++m_ActiveDotCount;
        if (dot)
        {
            dot->Activate(position, dotScale, 1.0f);
        }
    }
}
