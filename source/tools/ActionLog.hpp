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

  /// @brief Validates that the event payload contains all required fields with correct types.
  /// @param eventPayload JSON object representing the event to validate.
  /// @return True if the payload is valid, false otherwise.
  bool CheckRequiredFields(const nlohmann::json& eventPayload);

 public:
  ActionLog() = delete; // Delete default constructor
  /// @brief Basic constructor taking a reference to a DataLog instance.
  /// @param log Reference to DataLog where events will be recorded.
  explicit ActionLog(DataLog& log) : dataLog(log) {}
  ~ActionLog() = default;

  /// @brief Log an event with the provided data.
  /// @param eventPayload JSON object containing event data.
  /// @return Success status of the logging operation.
  [[nodiscard]] LogEventStatus LogEvent(const nlohmann::json& eventPayload);
};
}  // namespace cse498
