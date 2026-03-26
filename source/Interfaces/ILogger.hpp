/**
 * @file ILogger.hpp
 * @brief Interface for logging agent actions and replaying simulations.
 **/

#pragma once

#include <string>
#include <vector>

#include "IActionLog.hpp"
#include "IReplayDriver.hpp"
#include "IOutputManager.hpp"

namespace cse498 {

/**
 * @class ILogger
 * @brief Interface for coordinating action logging and replay operations.
 *
 * Provides methods for logging action events from agents during simulation
 * and replaying previously recorded events from file.
 */
class ILogger {
 public:
  virtual ~ILogger() = default;

  /// @brief Replay recorded events from a file.
  /// @param filePath Path to the JSON file containing logged events.
  /// @return true if replay was successful, false otherwise.
  virtual bool BeginReplay(const std::string& filePath) = 0;

  /// @brief Extract and validate action events from agents.
  /// @param agents Vector of agents to extract actions from.
  virtual void ExtractAgentActions(const std::vector<AgentType>& agents) = 0;
};

}  // namespace cse498
