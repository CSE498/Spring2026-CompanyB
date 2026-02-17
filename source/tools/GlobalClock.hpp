/**
 * @file GlobalClock.hpp
 * @author Abdulrahman Alanazi
 **/

#pragma once

#include <cstdint>

/* Class Description:
 *  Global clock for tracking simulation time.
 *  It is a static utility class and cannot be instantiated.
 *  It can be used by calling the static methods directly, e.g.
 * GlobalClock::GetTime() or GlobalClock::Tick(). Citation: Used AI responsibly
 * and actively in building the class below.
 */
namespace cse498 {

class GlobalClock {
 public:
  // GlobalClock is a static utility class and cannot be instantiated
  GlobalClock() = delete;

  // Get current global time in ticks
  static uint64_t GetTime();

  // Advance global time by a specified amount
  static void Tick(uint64_t deltaTime = 1);

  // Reset global time to zero
  static void Reset();

 private:
  // The current global time
  static uint64_t mGlobalTime;
};

}  // namespace cse498
