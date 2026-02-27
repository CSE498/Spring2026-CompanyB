#include "GlobalClock.hpp"

#include <cassert>

namespace cse498 {

// Define and initialize the static time counter to zero
uint64_t GlobalClock::mGlobalTime = 0;

uint64_t GlobalClock::GetTime() {
  // Returns the current value of the global time counter
  return mGlobalTime;
}

void GlobalClock::Tick(uint64_t deltaTime) {
  // Assert that the global time does not overflow
  assert(mGlobalTime + deltaTime >= mGlobalTime);
  // Increments the global time counter by the specified delta time amount
  mGlobalTime += deltaTime;
}

void GlobalClock::Reset() {
  // Resets the global time counter back to zero
  mGlobalTime = 0;
}

}  // namespace cse498
