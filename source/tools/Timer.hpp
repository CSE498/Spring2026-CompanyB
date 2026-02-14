/**
 * @file Timer.hpp
 * @brief A tool to make precise timing measurements.
 * @note Status: IN PROGRESS
 **/

#pragma once

#include <string>

namespace cse498 {

class Timer {
public:
    Timer(const std::string& name);

    void Start();
    void Stop();
    void Pause();
    void Resume();
    void Reset();

    double GetTotalTime() const;
    double GetTimeInSeconds() const;
    double GetTimeInMinutes() const;
    double GetTimeInHours() const;

    const std::string& GetName() const;

private:
    std::string name;
    double startTime;
    double endTime;
    double accumulatedTime;
    bool isRunning;
    bool isPaused;
    double pauseTime;
};

}
