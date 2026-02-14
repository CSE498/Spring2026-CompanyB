/**
 * This file is the interface for logging any events.
 * @brief API class for data logging.
 * @note Status: INITIAL DESIGN
 **/

#pragma once

#include "nlohmann/json.hpp"
#include "DataLog.hpp"

namespace cse498 {
    enum class LogEventStatus {
        SUCCESS,
        FAILURE
    };


    class ActionLog {
        private:
            /// @brief Reference to the DataLog instance where events will be recorded.
            DataLog& dataLog;
        public:
            /// @brief Basic constructor taking a reference to a DataLog instance.
            /// @param log Reference to DataLog where events will be recorded.
            ActionLog(DataLog& log) : dataLog(log) {}
            ~ActionLog() = default;

            /// @brief Log an event with the provided data.
            /// @param data JSON object containing event data.
            /// @return Success status of the logging operation.
            LogEventStatus LogEvent(const nlohmann::json& data);
    };

}