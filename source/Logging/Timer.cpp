#include "Timer.hpp"

namespace cse498 {

    uint64_t Timer::GetGlobalTime() const {
        return mGlobalTime;
    }

    // we may not need this function for now, but keeping it for future use
    void Timer::Tick(uint64_t deltaTime) {
        mGlobalTime += deltaTime;
    }

    void Timer::Reset() {
        mGlobalTime = 0;
    }

}
