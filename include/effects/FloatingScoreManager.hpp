#ifndef FLOATING_SCORE_MANAGER_HPP
#define FLOATING_SCORE_MANAGER_HPP

#include "Util/Color.hpp"
#include "Util/GameObject.hpp"

#include <functional>
#include <memory>
#include <string>

class FloatingScoreManager
{
public:
    using AddTimedElementFunction = std::function<void(const std::shared_ptr<Util::GameObject> &, float)>;

    void SpawnScore(const AddTimedElementFunction &addTimedElement,
                    const glm::vec2 &position,
                    int points,
                    const Util::Color &frontColor) const;
    void SpawnOutlinedText(const AddTimedElementFunction &addTimedElement,
                           const glm::vec2 &position,
                           const std::string &text,
                           const Util::Color &frontColor,
                           int fontSize,
                           float lifeTime,
                           const glm::vec2 &velocity) const;
};

#endif // FLOATING_SCORE_MANAGER_HPP
