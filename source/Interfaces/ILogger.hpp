/**
 * @file ILogger.hpp
 * @brief Interface for logging agent actions and replaying simulations.
 **/

#pragma once

#include <string>
#include <vector>

#include "IActionLog.hpp"
#include "IOutputManager.hpp"
#include "IReplayDriver.hpp"

namespace cse498 {

/**
 * @class ILogger
 * @brief Interface for coordinating action logging and replay operations.
 *
 * Provides methods for logging action events from agents during simulation
 * and replaying previously recorded events from file.
 */
template <typename AgentType>
class ILogger {
 public:
  virtual ~ILogger() = default;

  /// @brief Replay recorded events from a file.
  /// @param filePath Path to the JSON file containing logged events.
  /// @param agents Reference vector of agents updated during replay.
  /// @return true if replay was successful, false otherwise.
  virtual bool BeginReplay(const std::string& filePath,
                           std::vector<AgentType*>& agents) = 0;

  /// @brief Extract and validate action events from agents.
  /// @param agents Vector of agents to extract actions from.
  /// @param filePath Optional file path to save the logged events; if empty, a
  /// default path will be generated.
  virtual void SaveAgentActions(const std::vector<AgentType>& agents,
                                const std::string& filePath = "") = 0;
};

}  // namespace cse498
