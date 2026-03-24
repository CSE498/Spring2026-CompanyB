/**
 * @file IReplayDriver.hpp
 * @brief Interface for replaying recorded simulation events.
 **/

#pragma once

#include <string>

namespace cse498 {

/**
 * @class IReplayDriver
 * @brief Interface for replaying logged events from a file.
 *
 * Responsible for reading recorded agent actions from a file and replaying
 * them to reconstruct a previous simulation state.
 */
class IReplayDriver {
 public:
  virtual ~IReplayDriver() = default;

  /// @brief Replay logged events from a JSON file.
  /// @param filePath Path to the JSON file containing logged events.
  /// @return true if replay was successful, false otherwise.
  virtual bool ReplayFromFile(const std::string& filePath) = 0;
};

}  // namespace cse498
