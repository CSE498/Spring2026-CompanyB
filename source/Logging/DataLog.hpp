/**
 * 
 * @brief 
 * @note Status: INITIAL DESIGN
 **/

#pragma once

#include "nlohmann/json.hpp"

namespace cse498 {

    class DataLog {
        private:
        public:
            DataLog() = default;
            ~DataLog() = default;

            /// @brief 
            /// @param entry
            void AddEntry(const nlohmann::json& entry);
            // From Scotty: I made this function accept a JSON object but if you want to make a LogEvent struct I'm good with that
            // I just didn't know what fields that struct would have right now
    };

}