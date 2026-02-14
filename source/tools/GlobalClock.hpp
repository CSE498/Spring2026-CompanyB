/**
 * @file GlobalClock.hpp
 * @brief Global clock for tracking simulation time.
 * @note Status: INITIAL DESIGN
 **/

#pragma once

#include <cstdint>

namespace cse498 {

class GlobalClock {
public:
    /// @brief GlobalClock is a static utility class and cannot be instantiated.
    GlobalClock() = delete;

    /// @brief Get current global time in ticks.
    /// @return Global time as a 64-bit unsigned integer.
    static uint64_t GetTime();

    /// @brief Advance global time by a specified amount.
    /// @param deltaTime Amount to increment time by (in ticks).
    static void Tick(uint64_t deltaTime = 1);

    /// @brief Reset global time to zero.
    static void Reset();

private:
    /// @brief The global time counter in ticks.
    static uint64_t mGlobalTime;
};

} // namespace cse498
