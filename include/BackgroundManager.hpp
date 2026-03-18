#ifndef BACKGROUND_MANAGER_HPP
#define BACKGROUND_MANAGER_HPP

#include "BackgroundImage.hpp"
#include "Character.hpp"
#include "Util/GameObject.hpp"


#include <memory>
#include <vector>


class BackgroundManager : public Util::GameObject {
public:
  BackgroundManager() {}

private:
  std::vector<std::shared_ptr<BackgroundImage>> m_Backgrounds;
};

#endif // BACKGROUND_MANAGER_HPP