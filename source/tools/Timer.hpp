/**
 * @file Timer.hpp
 * @author Abdulrahman Alanazi
 **/

#pragma once

#include <cstdint>
#include <string>

namespace cse498 {
/* Class Description:
 *  A tool to make precise timing measurements. It should be told when to start
 * and stop, but also be giving a name for the timing, it acts as a stopwatch
 * that calculate the time in Ticks from GlobalClock It can also pause and
 * resume the timing measurements if needed. Citation: Used AI responsibly and
 * actively in building the class below.
 */

class Timer {
 public:
  /// Initialize the object with a @param name
  Timer(const std::string& name);
  // Start the stopwatch for the object
  void Start();
  // Stop the stopwatch for the object
  void Stop();
  // Pause the stopwatch for the object
  void Pause();
  // Resume the stopwatch for the object
  void Resume();
  // Reset the internal variables of the stopwatch for the object
  void Reset();
  /// @return total calculated time in ticks
  [[nodiscard]] uint64_t GetTotalTime() const;
  /// @return the given name of the object when it was initialized
  [[nodiscard]] const std::string& GetName() const;

 private:
  // Name of the object
  std::string mName;
  // Time when the stop watch is either started or resumed
  uint64_t mStartTime{0};
  // The accumulated total Time when the stopwatch is running, NOT including
  // when the stopwatch is paused
  uint64_t mAccumulatedTime{0};
  // Check if the stopwatch has started and not stopped
  bool mIsRunning{false};
  // Check if the stopwatch has been paused
  bool mIsPaused{false};
};

}  // namespace cse498
