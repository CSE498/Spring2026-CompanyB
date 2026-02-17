/**
 * @file Timer.hpp
 * @author Abdulrahman Alanazi
 **/

#pragma once

#include <string>



namespace cse498 {
    /* Class Description:
     *  A tool to make precise timing measurements. It should be told when to start and stop,
     *  but also be giving a name for the timing,
     *  it can track multiple different times and compare them as needed.
     *  It can also pause and resume the timing measurements if needed.
     *  Citation: Used AI responsibly and actively in building the class below.
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
        /// @return total calculated time while the stopwatch is running
        double GetTotalTime() const;
        /// @return  total calculated time in seconds
        double GetTimeInSeconds() const;
        /// @return total calculated time in minutes
        double GetTimeInMinutes() const;
        ///@return total calculated time in hours
        double GetTimeInHours() const;
        /// @return the given name of the object when it was initialized
        const std::string& GetName() const;

 private:
  // Name of the object
  std::string name;
  // Time when the stop watch is either started or resumed
  double startTime;
  // Time when the stop watch is stopped
  double endTime;
  // The accumulated total Time when the stopwatch is running, NOT including
  // when the stopwatch is paused
  double accumulatedTime;
  // Check if the stopwatch has started and not stopped
  bool isRunning;
  // Check if the stopwatch has been paused
  bool isPaused;

    double pauseTime;
};

}
