/**
 * This file is for the reading and replaying of a save file.
 * @brief API class for data replay.
 * @note Status: INITIAL DESIGN
 **/

#pragma once

#include "nlohmann/json.hpp"

namespace cse498 {
    class ReplayDriver {
        private:
            //world* world; need a pointer to the world object to instuct addtions and deletions of agents from world
        public:
            /// @brief Basic constructor for the ReplayDriver.
            ReplayDriver() = default;
            ~ReplayDriver() = default;

            /// @brief Method to replay logged events from a JSON file.
            /// @param filePath Path to the JSON file containing logged events.
            /// @return Success status of the replay operation.
            bool ReplayFromFile(const std::string& filePath);
            void SendInstructions(const nlohmann::json& eventData);
    };
}