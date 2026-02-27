#include "Timer.hpp"

#include "GlobalClock.hpp"

namespace cse498 {
// Constructor with the initialized member variables
Timer::Timer(const std::string &name)
    : mName(name),
      mStartTime(0.0),
      mAccumulatedTime(0.0),
      mIsRunning(false),
      mIsPaused(false) {}

void Timer::Start() {
  // If statement to prevent calling Start multiple times
  if (!mIsRunning) {
    mIsRunning = true;
    mIsPaused = false;
    mAccumulatedTime = 0.0;
    mStartTime = static_cast<double>(GlobalClock::GetTime());
  }
}

void Timer::Stop() {
  // If statement to prevent calling Stop multiple times
  if (mIsRunning) {
    const double endTime = static_cast<double>(GlobalClock::GetTime());
    if (!mIsPaused) {
      // Calculate total accumulated time while running
      mAccumulatedTime += (endTime - mStartTime);
    }
    mIsRunning = false;
    mIsPaused = false;
  }
}

void Timer::Pause() {
  // Check guards to verify it has started and is not already paused
  if (mIsRunning && !mIsPaused) {
    const double pauseTime = static_cast<double>(GlobalClock::GetTime());
    mAccumulatedTime += (pauseTime - mStartTime);
    mIsPaused = true;
  }
}

void Timer::Resume() {
  // Check guards to verify it has paused and is not already running
  if (mIsRunning && mIsPaused) {
    mStartTime = static_cast<double>(
        GlobalClock::GetTime());  // Reset start time for the new interval
    mIsPaused = false;
  }
}

void Timer::Reset() {
  mStartTime = 0.0;
  mAccumulatedTime = 0.0;
  mIsRunning = false;
  mIsPaused = false;
}

double Timer::GetTotalTime() const {
  if (!mIsRunning) {
    return mAccumulatedTime;
  }
  if (mIsPaused) {
    return mAccumulatedTime;
  }
  // If running and not paused, add the current interval to the accumulated time
  return mAccumulatedTime +
         (static_cast<double>(GlobalClock::GetTime()) - mStartTime);
}

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

const std::string &Timer::GetName() const { return mName; }

}
