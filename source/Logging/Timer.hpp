/**
 * @file Timer.hpp
 * @brief Global timer for tracking simulation time.
 * @note Status: INITIAL DESIGN
 **/

#pragma once

#include <cstdint>

namespace cse498 {

    class Timer {
        private:
            /// @brief Global time counter in ticks/milliseconds
            uint64_t mGlobalTime = 0;
            
        public:
            /// @brief Default constructor
            Timer() = default;
            
            /// @brief Destructor
            ~Timer() = default;
            
            /// @brief Copy constructor (disabled)
            Timer(const Timer&) = delete;
            
            /// @brief Assignment operator (disabled)
            Timer& operator=(const Timer&) = delete;
            
            /// @brief Get current global time
            /// @return Global time as integer (ticks/milliseconds)
            uint64_t GetGlobalTime() const;
            
            /// @brief Advance global time by specified amount
            /// @param deltaTime Amount to increment time by
            void Tick(uint64_t deltaTime);
            // from divya: not sure if we need this for now. i just thought of while writing the class. 
            
            /// @brief Reset global time to zero
            void Reset();
    };

}
