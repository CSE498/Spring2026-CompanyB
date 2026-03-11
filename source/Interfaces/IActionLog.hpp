/**
 * @file IActionLog.hpp
 * @author Scott Haakenson
 **/

#pragma once

#include <concepts>
#include <string_view>
#include <string>
#include <cstdint>
#include <memory>
#include <vector>

namespace cse498 {

/// @brief Severity of log messages
enum class LogLevel {
  Normal,
  Verbose,
  Debug,
  Silent
};

/// @brief Base struct for action events, containing common fields.
struct ActionEventBase {
  std::string_view agentId;
  std::string_view actionType;
  LogLevel logLevel;
  uint64_t timestamp;
};
/// @brief Struct for action events, generic for action-specific details.
/// @tparam Details 
template<typename Details>
struct ActionEvent : ActionEventBase {
    Details details;
};

/// @brief Status of a failed log event
struct LogEventFailure {
  const ActionEventBase& event;
  std::string message;
};

template<typename AgentType>
concept AgentConcept =
  requires(AgentType a) {
    { a.GetActions() };
  };

/**
 * @class IActionLog
 * @brief Interface for action logging.
 */
template<typename AgentType>
class IActionLog {
public:
  virtual ~IActionLog() = default;

  /// @brief Log actions performed by all agents of a world
  /// @param agents List of agents in the world
  /// @return Status of the logging operation
  virtual std::unique_ptr<std::vector<LogEventFailure>> LogAgentActions(
    const std::vector<AgentType>& agents) = 0;
};

}  // namespace cse498
