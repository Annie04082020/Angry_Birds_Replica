#ifndef BIRD_TRAIL_HPP
#define BIRD_TRAIL_HPP

#include "Util/GameObject.hpp"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

class BirdLaunchController;
class Character;

class BirdTrail
{
public:
    using AddElementFunction = std::function<void(const std::shared_ptr<Util::GameObject> &)>;

    void Build(const AddElementFunction &addElement);
    void Reset();
    void Update(const std::shared_ptr<BirdLaunchController> &birdLaunchController);

private:
    std::vector<std::shared_ptr<Util::GameObject>> m_Dots;
    std::unordered_map<const Character *, glm::vec2> m_LastEmitPositions;
    size_t m_ActiveDotCount = 0;
    int m_LastLaunchSequence = 0;
};

#endif // BIRD_TRAIL_HPP
