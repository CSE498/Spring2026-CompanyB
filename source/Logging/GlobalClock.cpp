#include "GlobalClock.hpp"

namespace cse498 {

// Define and initialize the static time counter.
uint64_t GlobalClock::mGlobalTime = 0;

uint64_t GlobalClock::GetTime() {
    return mGlobalTime;
}

void GlobalClock::Tick(uint64_t deltaTime) {
    mGlobalTime += deltaTime;
}

void GlobalClock::Reset() {
    mGlobalTime = 0;
}

} // namespace cse498
