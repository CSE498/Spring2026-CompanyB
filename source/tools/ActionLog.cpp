#include <cassert>
#include <string>

#include "ActionLog.hpp"
#include "DataLog.hpp"

namespace cse498 {

    static const std::array<std::string, 4> log_levels = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR"
    };

    bool CheckRequiredFields(const nlohmann::json& data) {
        // // Timestamp must be a non-negative integer
        // if (!data.contains("timestamp") || 
        //     !data["timestamp"].is_number_integer() ||
        //     data["timestamp"].get<int64_t>() <= 0
        // ) {
        //     return false;
        // }
        
        // Event type must be a non-empty string
        if (!data.contains("event_type") || 
            !data["event_type"].is_string() ||
            data["event_type"].get<std::string>().empty()
        ) {
            return false;
        }

        // Log level must be one of the predefined log levels
        if (!data.contains("log_level") || 
            !data["log_level"].is_string() ||
            (std::find(log_levels.begin(), log_levels.end(), data["log_level"].get<std::string>()) == log_levels.end())
        ) {
            return false;
        }

        // Details must be a JSON object
        if (!data.contains("details") || 
            !data["details"].is_object()
        ) {
            return false;
        }

        return true;
    }

    LogEventStatus ActionLog::LogEvent(const nlohmann::json& data) {
        if (!CheckRequiredFields(data)) {
            return LogEventStatus::FAILURE;
        }
        dataLog.AddEntry(data);
        return LogEventStatus::SUCCESS;
    }
}
