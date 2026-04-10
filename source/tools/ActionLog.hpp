/**
 * @file ActionLog.hpp
 * @author Scott Haakenson
 **/

#pragma once

#include <optional>

#include "../Interfaces/IActionLog.hpp"

namespace cse498 {

/// @brief This file is the interface for logging any events.
template <typename AgentType>
class ActionLog : public IActionLog<AgentType> {
 private:
  /// @brief Validates that the event payload contains all required fields with
  /// correct types.
  /// @param eventPayload ActionEventBase object representing the event to
  /// validate.
  /// @return True if the payload is valid, false otherwise.
  std::optional<LogEventFailure> ValidateBase(
      const ActionEventBase& eventPayload) {
    // agentId validation
    if (eventPayload.agentId.empty()) [[unlikely]] {
      return LogEventFailure{eventPayload,
                             "Validation failed: agentId is empty."};
    }
    // actionType validation
    if (eventPayload.actionType.empty()) [[unlikely]] {
      return LogEventFailure{eventPayload,
                             "Validation failed: actionType is empty."};
    }
    // logLevel validation
    if (eventPayload.logLevel < LogLevel::Normal ||
        eventPayload.logLevel > LogLevel::Silent) [[unlikely]] {
      return LogEventFailure{eventPayload,
                             "Validation failed: logLevel is out of range."};
    }
    return std::nullopt;
  }

 public:
  ActionLog() = default;
  ~ActionLog() = default;

  /// @brief Logs actions performed by all agents of a world
  /// @param agents List of agents in the world
  /// @return Flattened list of events from all agents that passed validation
  /// checks
  [[nodiscard]] std::pair<std::vector<ActionEventBase>,
                          std::vector<LogEventFailure>>
  LogAgentActions(const std::vector<AgentType>& agents) override {
    std::vector<ActionEventBase> events;
    std::vector<LogEventFailure> eventFailures;
    for (const auto& agent : agents) {
      const auto& actions = agent.GetActions();
      for (const auto& action : actions) {
        auto validationResult = ValidateBase(action);
        if (validationResult.has_value()) {
          eventFailures.push_back(validationResult.value());
          continue;
        }
        events.push_back(action);
      }
    }
    return {events, eventFailures};
  }
};

}  // namespace cse498
