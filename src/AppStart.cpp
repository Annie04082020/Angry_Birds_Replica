#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Logger.hpp"

void App::Start() {
    LOG_TRACE("Start");
    m_bird = std::make_shared<Character>(RESOURCE_DIR "/temp/R.png");

    
    m_CurrentState = State::UPDATE;
    
}


void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}
