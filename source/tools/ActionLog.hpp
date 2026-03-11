/**
 * @file ActionLog.hpp
 * @author Scott Haakenson
 **/

#pragma once

#include <optional>

#include "../Interfaces/IActionLog.hpp"
#include "../Interfaces/IDataLog.hpp"

namespace cse498 {

/// @brief This file is the interface for logging any events.
template<typename AgentType>
class ActionLog : public IActionLog<AgentType> {
 private:
  /// @brief Reference to the DataLog instance where events will be recorded.
  IDataLog& dataLog;

  /// @brief Validates that the event payload contains all required fields with correct types.
  /// @param eventPayload ActionEventBase object representing the event to validate.
  /// @return True if the payload is valid, false otherwise.
  std::optional<LogEventFailure> ValidateBase(const ActionEventBase& eventPayload) {
    // agentId validation
    if (eventPayload.agentId.empty()) {
      return LogEventFailure{eventPayload, "Validation failed: agentId is empty."};
    }
    // actionType validation
    if (eventPayload.actionType.empty()) {
      return LogEventFailure{eventPayload, "Validation failed: actionType is empty."};
    }
    // logLevel validation
    if (eventPayload.logLevel < LogLevel::Normal || eventPayload.logLevel > LogLevel::Silent) {
      return LogEventFailure{eventPayload, "Validation failed: logLevel is out of range."};
    }
    // timestamp validation
    if (eventPayload.timestamp < 0) {
      return LogEventFailure{eventPayload, "Validation failed: timestamp is negative."};
    }
    return std::nullopt;  // Placeholder for actual validation logic
  }

 public:
  ActionLog() = delete; // Delete default constructor
  /// @brief Basic constructor taking a reference to a DataLog instance.
  /// @param log Reference to DataLog where events will be recorded.
  explicit ActionLog(IDataLog& log) : dataLog(log) {}
  ~ActionLog() = default;

  /// @brief Logs actions performed by all agents of a world
  /// @param agents List of agents in the world
  /// @return Pointer to a vector of failed log events
  [[nodiscard]] std::unique_ptr<std::vector<LogEventFailure>> 
    LogAgentActions(const std::vector<AgentType>& agents) override {
      auto failedEvents = std::make_unique<std::vector<LogEventFailure>>();
    for (const auto& agent : agents) {
      const auto& actions = agent.GetActions();
      for (const auto& action : actions) {
        auto validationResult = ValidateBase(action);
        if (validationResult.has_value()) {
          failedEvents->push_back(validationResult.value());
          continue;  // Skip logging this event
        }
        dataLog.AddEntry(action);
      }
    }
    return failedEvents;
  }
};

}  // namespace cse498
