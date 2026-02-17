#include "ActionLog.hpp"

#include <cassert>
#include <string>

#include "DataLog.hpp"
#include "OutputManager.hpp"

using cse498::LogLevel;

namespace cse498 {
bool CheckRequiredFields(const nlohmann::json &data) {
  // Event type must be a non-empty string
  if (!data.contains("type") || !data["type"].is_string() ||
      data["type"].get<std::string>().empty()) {
    return false;
  }

  // Log level must be one of the predefined log levels
  if (!data.contains("log_level") || !data["log_level"].is_number_integer()) {
    return false;
  }
  int logLevelInt = data["log_level"].get<int>();
  if (logLevelInt < static_cast<int>(LogLevel::Silent) ||
      logLevelInt > static_cast<int>(LogLevel::Debug)) {
    return false;
  }

  // ID must be a non-empty string
  if (!data.contains("id") ||
      !data["id"].is_string() ||
      data["id"].get<std::string>().empty()) {
    return false;
  }

  // Details must be a JSON object
  if (!data.contains("details") || !data["details"].is_object()) {
    return false;
  }

  return true;
}

LogEventStatus ActionLog::LogEvent(const nlohmann::json &data) {
  if (!CheckRequiredFields(data)) {
    return LogEventStatus::FAILURE;
  }
  dataLog.AddEntry(data);
  return LogEventStatus::SUCCESS;
}
}  // namespace cse498
