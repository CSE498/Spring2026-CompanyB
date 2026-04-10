/**
 * @file IActionLog.hpp
 * @author Scott Haakenson
 **/

#pragma once

#include <concepts>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace cse498 {

/// @brief Severity of log messages
enum class LogLevel { Normal, Verbose, Debug, Silent };

/// @brief Base struct for action events, containing common fields.
struct ActionEventBase {
  /// @brief Unique identifier for the agent performing the action.
  std::string_view agentId = "";
  /// @brief Type of action being performed; indicates details type.
  std::string_view actionType = "";
  /// @brief Severity level of the log message.
  LogLevel logLevel = LogLevel::Silent;
  /// @brief Timestamp of when the action occurred (determined by world ticks).
  uint64_t timestamp = 0;
};
/// @brief Struct for action events, generic for action-specific details.
/// @tparam Details Action-specific details type, allowing for flexible event
/// payloads.
template <typename Details>
struct ActionEvent : ActionEventBase {
  /// @brief Action-specific details, allowing for flexible event payloads.
  Details details;
};

/// @brief Status of a failed log event
struct LogEventFailure {
  /// @brief Reference to the failed event to provide context for debugging.
  const ActionEventBase& event;
  /// @brief Message explaining the reason for the failure.
  std::string message = "";
};

template <typename AgentType>
concept AgentConcept = requires(AgentType a) {
  { a.GetActions() };
};

/**
 * @class IActionLog
 * @brief Interface for action logging.
 */
template <AgentConcept AgentType>
class IActionLog {
 public:
  virtual ~IActionLog() = default;

  /// @brief Log actions performed by all agents of a world
  /// @param agents List of agents in the world
  /// @return Pair of vectors: first contains events that passed validation
  /// checks, second contains details of events that failed validation
  virtual std::pair<std::vector<ActionEventBase>, std::vector<LogEventFailure>>
  LogAgentActions(const std::vector<AgentType>& agents) = 0;
};

}  // namespace cse498
