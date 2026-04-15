#include "TickTimer.hpp"

#include <cassert>

#include "GlobalClock.hpp"
namespace cse498 {
// Constructor with the initialized member variables
TickTimer::TickTimer(const std::string& name) : mName(name) {}

void TickTimer::Start() {
  // Assert to prevent calling Start multiple times
  assert(!mIsRunning && "TickTimer is already running");

  mIsRunning = true;
  mIsPaused = false;
  mAccumulatedTime = 0;
  mStartTime = GlobalClock::GetTime();
}

void TickTimer::Stop() {
  // Assert to prevent calling Stop multiple times
  assert(mIsRunning && "TickTimer is not running");

  const uint64_t endTime = GlobalClock::GetTime();
  if (!mIsPaused) {
    // Calculate total accumulated time while running
    mAccumulatedTime += (endTime - mStartTime);
  }
  mIsRunning = false;
  mIsPaused = false;
}

void TickTimer::Pause() {
  // Assert guards to verify it has started and is not already paused
  assert(mIsRunning && "TickTimer is not running");
  assert(!mIsPaused && "TickTimer is already paused");

  const uint64_t pauseTime = GlobalClock::GetTime();
  mAccumulatedTime += (pauseTime - mStartTime);
  mIsPaused = true;
}

void TickTimer::Resume() {
  // Assert guards to verify it has paused and is not already running
  assert(mIsRunning && "TickTimer is not running");
  assert(mIsPaused && "TickTimer is not paused");

  mStartTime = GlobalClock::GetTime();  // Reset start time for the new interval
  mIsPaused = false;
}

void TickTimer::Reset() {
  // Reset the internal variables of the stopwatch
  *this = TickTimer{mName};
}

uint64_t TickTimer::GetTotalTime() const {
  if (!mIsRunning || mIsPaused) {
    return mAccumulatedTime;
  }
  // If running and not paused, add the current interval to the accumulated time
  return mAccumulatedTime + (GlobalClock::GetTime() - mStartTime);
}

const std::string& TickTimer::GetName() const { return mName; }

}  // namespace cse498
