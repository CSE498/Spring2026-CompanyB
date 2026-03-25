/**
 * @file ILogger.hpp
 * @brief Interface for logging agent actions and replaying simulations.
 **/

#pragma once

#include <string>
#include <vector>

#include "IActionLog.hpp"
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

  /// @brief Save action events to a file.
  /// @param events Vector of ActionEventBase objects to be saved.
  /// @return true if successfully saved to file through output manager, false
  /// otherwise.
  virtual bool SaveToFile(const std::vector<ActionEventBase>& events) = 0;
};

}  // namespace cse498
