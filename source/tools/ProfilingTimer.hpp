#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace cse498 {

/** Class Description:
 *  A tool to make precise timing measurements using std::chrono::steady_clock.
 *  It should be told when to start and stop, but also be given a name for the
 *  timing. It acts as a stopwatch that calculates the time in nanoseconds.
 *  It can also pause and resume the timing measurements if needed.
 *  Used similar design to what in TickTimer.
 */
class ProfilingTimer {
 public:
  /// Initialize the object with a @param name
  ProfilingTimer(const std::string& name);
  /// Start the stopwatch for the object
  void Start();
  /// Stop the stopwatch for the object
  void Stop();
  /// Pause the stopwatch for the object
  void Pause();
  /// Resume the stopwatch for the object
  void Resume();
  /// Reset the internal variables of the stopwatch for the object
  void Reset();
  /// @return total calculated time in nanoseconds
  [[nodiscard]] uint64_t GetTotalTime() const;
  /// @return the given name of the object when it was initialized
  [[nodiscard]] const std::string& GetName() const;

 private:
  /// Name of the object
  std::string mName;
  /// Time when the stopwatch is either started or resumed
  std::chrono::steady_clock::time_point mStartTime{};
  /// The accumulated total Time when the stopwatch is running, NOT including
  /// when the stopwatch is paused
  std::chrono::steady_clock::duration mAccumulatedTime{0};
  /// Check if the stopwatch has started and not stopped
  bool mIsRunning{false};
  /// Check if the stopwatch has been paused
  bool mIsPaused{false};
};

}  // namespace cse498