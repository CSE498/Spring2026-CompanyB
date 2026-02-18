/**
 * @file ActionLog.hpp
 * @author Scott Haakenson
 **/

#pragma once

#include "DataLog.hpp"
#include "nlohmann/json.hpp"

namespace cse498 {
/// @brief Status codes for logging events
enum class LogEventStatus { SUCCESS, FAILURE };

/// @brief This file is the interface for logging any events.
class ActionLog {
 private:
  /// @brief Reference to the DataLog instance where events will be recorded.
  DataLog& dataLog;

 public:
  ActionLog() = delete; // Delete default constructor
  /// @brief Basic constructor taking a reference to a DataLog instance.
  /// @param log Reference to DataLog where events will be recorded.
  ActionLog(DataLog& log) : dataLog(log) {}
  ~ActionLog() = default;

  /// @brief Log an event with the provided data.
  /// @param data JSON object containing event data.
  /// @return Success status of the logging operation.
  LogEventStatus LogEvent(const nlohmann::json& data);
};
}  // namespace cse498
