#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Logger.hpp"

void App::Start() {
    LOG_TRACE("Start");


    
    m_CurrentState = State::UPDATE;
    
}


void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}
