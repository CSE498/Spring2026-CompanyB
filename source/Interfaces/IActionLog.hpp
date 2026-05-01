/**
 * @file IActionLog.hpp
 * @author Scott Haakenson
 **/

#pragma once

#include <concepts>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

namespace cse498 {

/// @brief Severity of log messages
enum class LogLevel { Normal, Verbose, Debug, Silent };

/// @brief Status of a failed log event
struct LogEventFailure {
  /// @brief Copy of the failed event to provide context for debugging.
  const nlohmann::json state;
  /// @brief Message explaining the reason for the failure.
  std::string message = "";
};

template <typename AgentType>
concept AgentConcept = requires(AgentType a) {
  { a.GetStates() };
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
  virtual std::pair<std::vector<nlohmann::json>, std::vector<LogEventFailure>>
  LogAgentStates(const std::vector<AgentType>& agents) = 0;
};

}  // namespace cse498
