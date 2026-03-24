/**
 * @file Logger.hpp
 * @brief Coordinates logging of actions and replay of recorded events.
 **/

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../Interfaces/IActionLog.hpp"
#include "../Interfaces/ILogger.hpp"
#include "../Interfaces/IOutputManager.hpp"
#include "../Interfaces/IReplayDriver.hpp"

namespace cse498 {

class AgentBase;

/**
 * @class Logger
 * @brief Implements action logging workflow and replay operations.
 *
 * Coordinates between ActionLog, OutputManager, and ReplayDriver to manage
 * the complete logging and replay lifecycle. During simulation, accepts action
 * events and persists them to file. Supports replaying previously recorded
 * events to reconstruct simulation states.
 */
class Logger : public ILogger {
 private:
  /// @brief Interface for extracting and validating agent actions.
  std::unique_ptr<IActionLog<AgentBase>> mActionLog;

  /// @brief Interface for persisting log data to file with auto-generated names.
  std::unique_ptr<IOutputManager> mOutputManager;

  /// @brief Interface for replaying recorded events from file.
  std::unique_ptr<IReplayDriver> mReplayDriver;

 public:
  /// @brief Constructs a Logger with action logging and output management.
  /// @param actionLog Unique pointer to IActionLog implementation.
  /// @param outputManager Unique pointer to IOutputManager implementation.
  /// @param replayDriver Optional unique pointer to IReplayDriver implementation.
  Logger(std::unique_ptr<IActionLog<AgentBase>> actionLog,
         std::unique_ptr<IOutputManager> outputManager,
         std::unique_ptr<IReplayDriver> replayDriver = nullptr)
      : mActionLog(std::move(actionLog)),
        mOutputManager(std::move(outputManager)),
        mReplayDriver(std::move(replayDriver)) {}

  ~Logger() override = default;

  // REPLAY:
  /// @brief Replay recorded events from a file.
  /// @param filePath Path to the JSON file containing logged events.
  /// @return true if replay was successful, false if no ReplayDriver is set.
  bool BeginReplay(const std::string& filePath) override {
    if (!mReplayDriver) {
      return false;
    }

    return mReplayDriver->ReplayFromFile(filePath);
  }

  // SAVING DURING LIVE SIMULATION:
  // to do
  // to implement a method that calls ActionLog's function 
  // and returns a vector of ActionEventBase objects that 
  // can be passed to the Logger's LogActionEvents method 
  // (can make a call to this method inside the func to do).

  /// @brief Log action events from agents and persist to output.
  /// @param events Vector of ActionEventBase objects to be logged.
  /// @return true if successfully logged to output manager, false if no
  /// OutputManager is set.
  bool SaveToFile(const std::vector<ActionEventBase>& events) override {
    if (!mOutputManager) {
      return false;
    }
    mOutputManager->WriteActionEvents(events);
    return true;
  }
};

}  // namespace cse498
