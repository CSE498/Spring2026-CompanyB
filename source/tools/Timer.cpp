#include "Timer.hpp"

#include <stdexcept>

#include "GlobalClock.hpp"

namespace cse498 {
// Constructor with the initialized member variables
Timer::Timer(const std::string &name)
    : name(name),
      startTime(0.0),
      endTime(0.0),
      accumulatedTime(0.0),
      isRunning(false),
      isPaused(false),
      pauseTime(0.0) {}

void Timer::Start() {
  // If statement to prevent calling Start multiple times
  if (!isRunning) {
    isRunning = true;
    isPaused = false;
    accumulatedTime = 0.0;
    startTime = static_cast<double>(GlobalClock::GetTime());
  }
}

void Timer::Stop() {
  // If statement to prevent calling Stop multiple times
  if (isRunning) {
    endTime = static_cast<double>(GlobalClock::GetTime());
    if (!isPaused) {
      // Calculate total accumulated time while running
      accumulatedTime += (endTime - startTime);
    }
    isRunning = false;
    isPaused = false;
  }
}

void Timer::Pause() {
  // Check guards to verify it has started and is not already paused
  if (isRunning && !isPaused) {
    pauseTime = static_cast<double>(GlobalClock::GetTime());
    accumulatedTime += (pauseTime - startTime);
    isPaused = true;
  }
}

void Timer::Resume() {
  // Check guards to verify it has paused and is not already running
  if (isRunning && isPaused) {
    startTime = static_cast<double>(
        GlobalClock::GetTime());  // Reset start time for the new interval
    isPaused = false;
  }
}

void Timer::Reset() {
  startTime = 0.0;
  endTime = 0.0;
  accumulatedTime = 0.0;
  isRunning = false;
  isPaused = false;
  pauseTime = 0.0;
}

double Timer::GetTotalTime() const {
  if (!isRunning) {
    return accumulatedTime;
  }
  if (isPaused) {
    return accumulatedTime;
  }
  // If running and not paused, add the current interval to the accumulated time
  return accumulatedTime +
         (static_cast<double>(GlobalClock::GetTime()) - startTime);
}

double Timer::GetTimeInSeconds() const {
  // Assuming 1 tick = 1 millisecond
  return GetTotalTime() / 1000.0;
}

double Timer::GetTimeInMinutes() const { return GetTimeInSeconds() / 60.0; }

double Timer::GetTimeInHours() const { return GetTimeInMinutes() / 60.0; }

const std::string &Timer::GetName() const { return name; }

}  // namespace cse498
