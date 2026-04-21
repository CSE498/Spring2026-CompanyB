#include "ProfilingTimer.hpp"

#include <cassert>

namespace cse498 {

// Constructor with the initialized member variables
ProfilingTimer::ProfilingTimer(const std::string& name) : mName(name) {}

void ProfilingTimer::Start() {
  // Assert to prevent calling Start multiple times
  assert(!mIsRunning && "ProfilingTimer is already running");

  mIsRunning = true;
  mIsPaused = false;
  mAccumulatedTime = std::chrono::steady_clock::duration::zero();
  mStartTime = std::chrono::steady_clock::now();
}

void ProfilingTimer::Stop() {
  // Assert to prevent calling Stop multiple times
  assert(mIsRunning && "ProfilingTimer is not running");

  const auto endTime = std::chrono::steady_clock::now();
  if (!mIsPaused) {
    // Calculate total accumulated time while running
    mAccumulatedTime += (endTime - mStartTime);
  }
  mIsRunning = false;
  mIsPaused = false;
}

void ProfilingTimer::Pause() {
  // Assert guards to verify it has started and is not already paused
  assert(mIsRunning && "ProfilingTimer is not running");
  assert(!mIsPaused && "ProfilingTimer is already paused");

  const auto pauseTime = std::chrono::steady_clock::now();
  mAccumulatedTime += (pauseTime - mStartTime);
  mIsPaused = true;
}

void ProfilingTimer::Resume() {
  // Assert guards to verify it has paused and is not already running
  assert(mIsRunning && "ProfilingTimer is not running");
  assert(mIsPaused && "ProfilingTimer is not paused");

  mStartTime = std::chrono::steady_clock::now();  // Reset start time for the
                                                  // new interval
  mIsPaused = false;
}

void ProfilingTimer::Reset() {
  // Reset the internal variables of the stopwatch
  *this = ProfilingTimer{mName};
}

uint64_t ProfilingTimer::GetTotalTime() const {
  auto totalTime = mAccumulatedTime;
  if (mIsRunning && !mIsPaused) {
    totalTime += (std::chrono::steady_clock::now() - mStartTime);
  }
  return std::chrono::duration_cast<std::chrono::nanoseconds>(totalTime)
      .count();
}

const std::string& ProfilingTimer::GetName() const { return mName; }

}  // namespace cse498