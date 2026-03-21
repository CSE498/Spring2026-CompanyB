/**
 * @file GlobalClock.hpp
 * @author Abdulrahman Alanazi
 **/

#pragma once

#include <cstdint>

namespace cse498 {

/* Class Description:
 *  Global clock for tracking simulation time in ticks.
 *  It is a static utility class and cannot be instantiated.
 *  I expect this class to be used by everyone who need to know the current time
 * of simulation. It can be used by calling the static methods directly, e.g.
 * GlobalClock::GetTime() or GlobalClock::Tick(). You can call
 * GlobalClock::Tick() to make the time pass by 1 ticks, or to provide a
 * parameter for Tick() to make the time pass by that amount. GetTime() would
 * return the current time in ticks. Citation: Used AI to edit the initial
 * design I had of the class to make it static so it won't be constructed.
 */
class GlobalClock {
 public:
  // GlobalClock is a static utility class and cannot be instantiated
  GlobalClock() = delete;

  /// @return current global time in ticks
  static uint64_t GetTime();

  /// Advance global time by a @param deltaTime amount
  static void Tick(uint64_t deltaTime = 1);

  // Reset global time to zero
  static void Reset();

 private:
  // The current global time
  static uint64_t mGlobalTime;
};

}  // namespace cse498
