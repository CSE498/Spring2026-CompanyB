/**
 * @file IReplayDriver.hpp
 * @brief Interface for replaying recorded simulation events.
 **/

#pragma once

#include <expected>
#include <string>
#include <vector>

namespace cse498 {

/**
 * @class IReplayDriver
 * @brief Interface for replaying logged events from a file.
 *
 * Responsible for reading recorded agent actions from a file and replaying
 * them to reconstruct a previous simulation state.
 */
template <typename AgentType>
class IReplayDriver {
 public:
  virtual ~IReplayDriver() = default;

  /// @brief Replay logged events from a JSON file.
  /// @param filePath Path to the JSON file containing logged events.
  /// @param agents Reference vector of agents to update during replay.
  /// @return true if replay was successful, otherwise an error describing
  /// replay failure.
  virtual std::expected<bool, std::string> ReplayFromFile(
      const std::string& filePath, std::vector<AgentType*>& agents) = 0;
};

}  // namespace cse498
