#include "Timer.hpp"

#include "GlobalClock.hpp"
#include <cassert>
namespace cse498 {
// Constructor with the initialized member variables
Timer::Timer(const std::string &name)
    : mName(name) {}

void Timer::Start() {
  // Assert to prevent calling Start multiple times
  assert(!mIsRunning && "Timer is already running");

  mIsRunning = true;
  mIsPaused = false;
  mAccumulatedTime = 0;
  mStartTime = GlobalClock::GetTime();
}

void Timer::Stop() {
  // Assert to prevent calling Stop multiple times
  assert(mIsRunning && "Timer is not running");

  const uint64_t endTime = GlobalClock::GetTime();
  if (!mIsPaused) {
    // Calculate total accumulated time while running
    mAccumulatedTime += (endTime - mStartTime);
  }
  mIsRunning = false;
  mIsPaused = false;
}

void Timer::Pause() {
  // Assert guards to verify it has started and is not already paused
  assert(mIsRunning && "Timer is not running");
  assert(!mIsPaused && "Timer is already paused");

  const uint64_t pauseTime = GlobalClock::GetTime();
  mAccumulatedTime += (pauseTime - mStartTime);
  mIsPaused = true;
}

void Timer::Resume() {
  // Assert guards to verify it has paused and is not already running
  assert(mIsRunning && "Timer is not running");
  assert(mIsPaused && "Timer is not paused");

  mStartTime = GlobalClock::GetTime();  // Reset start time for the new interval
  mIsPaused = false;
}

void Timer::Reset() {
  // Reset the internal variables of the stopwatch
  *this = Timer{mName};
}

uint64_t Timer::GetTotalTime() const {
  if (!mIsRunning || mIsPaused) {
    return mAccumulatedTime;
  }
  // If running and not paused, add the current interval to the accumulated time
  return mAccumulatedTime +
         (GlobalClock::GetTime() - mStartTime);
}

/*
double Timer::GetTimeInSeconds() const {
  // Assuming 1 tick = 1 millisecond
  constexpr double millisecondsPerSecond = 1000.0;
  return GetTotalTime() / millisecondsPerSecond;
}

double Timer::GetTimeInMinutes() const {
  constexpr double secondsPerMinute = 60.0;
  return GetTimeInSeconds() / secondsPerMinute;
}

double Timer::GetTimeInHours() const {
  constexpr double minutesPerHour = 60.0;
  return GetTimeInMinutes() / minutesPerHour;
}
*/

const std::string& Timer::GetName() const { return mName; }

}
