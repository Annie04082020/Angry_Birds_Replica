#include "DynamicBackground.hpp"
#include "BackgroundImage.hpp"

DynamicBackground::DynamicBackground()
    : m_Background(RESOURCE_DIR "temp/test_bg.png") {}

void DynamicBackground::Update(float dt) { m_Background->Update(dt); }
