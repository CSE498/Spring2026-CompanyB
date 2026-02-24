#include "ActionLog.hpp"

#include <cassert>
#include <string>

#include "DataLog.hpp"
#include "OutputManager.hpp"

using cse498::LogLevel;

namespace cse498 {
bool ActionLog::CheckRequiredFields(const nlohmann::json &eventPayload) {
  // Event type must be a non-empty string
  if (!eventPayload.contains("type") || !eventPayload["type"].is_string() ||
      eventPayload["type"].get<std::string>().empty()) {
    return false;
  }

  // Log level must be one of the predefined log levels
  if (!eventPayload.contains("log_level") || !eventPayload["log_level"].is_number_integer()) {
    return false;
  }
  int logLevelInt = eventPayload["log_level"].get<int>();
  if (logLevelInt < static_cast<int>(LogLevel::Silent) ||
      logLevelInt > static_cast<int>(LogLevel::Debug)) {
    return false;
  }

  // ID must be a non-empty string
  if (!eventPayload.contains("id") ||
      !eventPayload["id"].is_string() ||
      eventPayload["id"].get<std::string>().empty()) {
    return false;
  }

  // Details must be a JSON object
  if (!eventPayload.contains("details") || !eventPayload["details"].is_object()) {
    return false;
  }

  return true;
}

LogEventStatus ActionLog::LogEvent(const nlohmann::json &eventPayload) {
  if (!eventPayload.is_object()) {  // Check if payload is not-null JSON object (dictionary)
    return LogEventStatus::FAILURE;
  }
  if (!CheckRequiredFields(eventPayload)) {
    return LogEventStatus::FAILURE;
  }
  dataLog.AddEntry(eventPayload);
  return LogEventStatus::SUCCESS;
}
}  // namespace cse498
