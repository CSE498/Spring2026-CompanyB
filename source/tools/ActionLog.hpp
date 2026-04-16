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
  /// @param eventPayload JSON object representing the event to
  /// validate.
  /// @return True if the payload is valid, false otherwise.
  std::optional<LogEventFailure> ValidateBase(const nlohmann::json& eventPayload) {
    // agentId validation
    if (!eventPayload.contains("agentId") || 
        !eventPayload.operator[]("agentId").is_string() || 
        eventPayload.operator[]("agentId").get<std::string>().empty()) [[unlikely]] {
      return LogEventFailure{eventPayload, "Validation failed: agentId is missing or empty."};
    }
    // logLevel validation
    if (!eventPayload.contains("logLevel") || 
        !eventPayload.operator[]("logLevel").is_number_integer()) [[unlikely]] {
      return LogEventFailure{eventPayload, "Validation failed: logLevel is missing or not an integer."};
    }
    int logLevel = eventPayload.operator[]("logLevel").get<int>();
    if (logLevel < static_cast<int>(LogLevel::Normal) || 
        logLevel > static_cast<int>(LogLevel::Silent)) [[unlikely]] {
      return LogEventFailure{eventPayload, "Validation failed: logLevel is out of range."};
    }
    // timestamp validation
    if (!eventPayload.contains("timestamp") || 
        !eventPayload.operator[]("timestamp").is_number_integer() || eventPayload.operator[]("timestamp").get<int64_t>() <= 0) [[unlikely]] {
      return LogEventFailure{eventPayload, "Validation failed: timestamp is missing or not positive."};
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
  [[nodiscard]] std::pair<std::vector<nlohmann::json>,
                          std::vector<LogEventFailure>>
  LogAgentStates(const std::vector<AgentType>& agents) override {
    std::vector<nlohmann::json> events;
    std::vector<LogEventFailure> eventFailures;
    for (const auto& agent : agents) {
      const auto& actions = agent.GetStates();
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
