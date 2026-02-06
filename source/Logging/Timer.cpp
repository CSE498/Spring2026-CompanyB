#include "Timer.hpp"
#include "GlobalClock.hpp"
#include <stdexcept>

namespace cse498 {

Timer::Timer(const std::string& name)
    : name(name), startTime(0.0), endTime(0.0), accumulatedTime(0.0),
      isRunning(false), isPaused(false), pauseTime(0.0) {}

void Timer::Start() {
    if (!isRunning) {
        isRunning = true;
        isPaused = false;
        accumulatedTime = 0.0;
        startTime = static_cast<double>(GlobalClock::GetTime());
    }
}

void Timer::Stop() {
    if (isRunning) {
        endTime = static_cast<double>(GlobalClock::GetTime());
        if (!isPaused) {
            accumulatedTime += (endTime - startTime);
        }
        isRunning = false;
        isPaused = false;
    }
}

void Timer::Pause() {
    if (isRunning && !isPaused) {
        pauseTime = static_cast<double>(GlobalClock::GetTime());
        accumulatedTime += (pauseTime - startTime);
        isPaused = true;
    }
}

void Timer::Resume() {
    if (isRunning && isPaused) {
        startTime = static_cast<double>(GlobalClock::GetTime()); // Reset start time for the new interval
        isPaused = false;
    }
}

void Timer::Reset() {
    name = "";
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
    return accumulatedTime + (static_cast<double>(GlobalClock::GetTime()) - startTime);
}

double Timer::GetTimeInSeconds() const {
    // Assuming 1 tick = 1 millisecond
    return GetTotalTime() / 1000.0;
}

double Timer::GetTimeInMinutes() const {
    return GetTimeInSeconds() / 60.0;
}

double Timer::GetTimeInHours() const {
    return GetTimeInMinutes() / 60.0;
}

const std::string& Timer::GetName() const {
    return name;
}

}
