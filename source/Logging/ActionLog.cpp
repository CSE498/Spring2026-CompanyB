#include <cassert>
#include <string>

#include "ActionLog.hpp"
#include "DataLog.hpp"

namespace cse498 {

    LogEventStatus ActionLog::LogEvent(const nlohmann::json& data) {
        // No enforcement or validation on data for now.
        dataLog.AddEntry(data);
        return LogEventStatus::SUCCESS;
    }

}