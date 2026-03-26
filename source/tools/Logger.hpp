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

  /// @brief Interface for persisting log data to file with auto-generated
  /// names.
  std::unique_ptr<IOutputManager> mOutputManager;

  /// @brief Interface for replaying recorded events from file.
  std::unique_ptr<IReplayDriver> mReplayDriver;

 public:
  /// @brief Constructs a Logger with action logging and output management.
  Logger() {
    mActionLog = std::make_unique<IActionLog<AgentBase>>();
    mOutputManager = std::make_unique<IOutputManager>();
    mReplayDriver = std::make_unique<IReplayDriver>();
  }

  ~Logger() override = default;

  // REPLAY:
  /// @brief Replay recorded events from a file.
  /// @param filePath Path to the JSON file containing logged events.
  /// @return true if replay was successful, false if no ReplayDriver is set.
  /// NOTE: need to have another param that takes in a ref to an empty container 
  /// from world to populate with agents and their action info for replay
  bool BeginReplay(const std::string& filePath) override {
    if (!mReplayDriver) {
      return false;
    }

    return mReplayDriver->ReplayFromFile(filePath);
  }

  // SAVING DURING LIVE SIMULATION:
  /// @brief Extract and validate action events from all agents.
  /// @param agents Vector of agents to extract actions from.
void ExtractAgentActions(const std::vector<AgentType>& agents) override {
    std::vector<ActionEventBase> events = mActionLog->LogAgentActions(agents);
    mOutputManager->WriteActionEvents(events);
  }
};

}  // namespace cse498
