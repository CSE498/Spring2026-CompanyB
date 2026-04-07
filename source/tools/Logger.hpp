/**
 * @file Logger.hpp
 * @brief Coordinates logging of actions and replay of recorded events.
 **/

#pragma once

#include <expected>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "../Interfaces/IActionLog.hpp"
#include "../Interfaces/ILogger.hpp"
#include "../Interfaces/IOutputManager.hpp"
#include "../Interfaces/IReplayDriver.hpp"
#include "ActionLog.hpp"
#include "OutputManager.hpp"
#include "ReplayDriver.hpp"
#include "nlohmann/json.hpp"

namespace cse498 {

/**
 * @class Logger
 * @brief Implements action logging workflow and replay operations.
 *
 * Coordinates between ActionLog, OutputManager, and ReplayDriver to manage
 * the complete logging and replay lifecycle. During simulation, accepts action
 * events and persists them to file. Supports replaying previously recorded
 * events to reconstruct simulation states.
 */
template <typename AgentType>
class Logger : public ILogger<AgentType> {
 private:
  /// @brief Interface for extracting and validating agent actions.
  std::unique_ptr<IActionLog<AgentType>> mActionLog;

  /// @brief Interface for persisting log data to file with auto-generated
  /// names.
  std::unique_ptr<IOutputManager> mOutputManager;

  /// @brief Interface for replaying recorded events from file.
  std::unique_ptr<IReplayDriver<AgentType>> mReplayDriver;

 public:
  /// @brief Constructs a logger with explicit dependencies.
  Logger()
      : mActionLog(std::make_unique<ActionLog<AgentType>>()),
        mOutputManager(std::make_unique<OutputManager>()),
        mReplayDriver(std::make_unique<ReplayDriver<AgentType>>()) {}

  /// @brief Constructs a logger with provided dependencies (for testing or
  /// custom implementations).
  /// @param actionLog Unique pointer to an IActionLog implementation.
  /// @param outputManager Unique pointer to an IOutputManager implementation.
  /// @param replayDriver Unique pointer to an IReplayDriver implementation.
  Logger(std::unique_ptr<IActionLog<AgentType>> actionLog,
         std::unique_ptr<IOutputManager> outputManager,
         std::unique_ptr<IReplayDriver<AgentType>> replayDriver)
      : mActionLog(std::move(actionLog)),
        mOutputManager(std::move(outputManager)),
        mReplayDriver(std::move(replayDriver)) {}

  /// @brief Deleted copy constructor and assignment operator to prevent
  /// copying.
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  /// @brief Defaulted move constructor
  Logger(Logger&&) = default;
  /// @brief Defaulted move assignment operator
  Logger& operator=(Logger&&) = default;

  ~Logger() override = default;

  // REPLAY:
  /// @brief Replay recorded events from a file.
  /// @param filePath Path to the JSON file containing logged events.
  /// @param agents Reference vector of agents updated during replay.
  /// @return true if replay loading was successful
  bool BeginReplay(const std::string& filePath,
                   std::vector<AgentType*>& agents) override {
    return mReplayDriver->ReplayFromFile(filePath, agents).value_or(false);
  }

  // SAVING DURING LIVE SIMULATION:
  /// @brief Extract and validate action events from all agents.
  /// @param agents Vector of agents to extract actions from.
  /// @return Empty expected on success; otherwise an error describing the
  /// failed stage.
  std::expected<void, SaveAgentActionsError> SaveAgentActions(
      const std::vector<AgentType>& agents) override {
    auto [events, eventFailures] = mActionLog->LogAgentActions(agents);
    for (const auto& failure : eventFailures) {
      mOutputManager->LogMessage(
          LogLevel::Verbose,
          "Validation failed for event: agentId=" +
              std::string(failure.event.agentId) + ", actionType=" +
              std::string(failure.event.actionType) + ", logLevel=" +
              std::to_string(static_cast<int>(failure.event.logLevel)) +
              ", timestamp=" + std::to_string(failure.event.timestamp) +
              ". Reason: " + failure.message);
    }
    mOutputManager->WriteActionEvents(events);
    if (!mOutputManager->Flush()) {
      return std::unexpected(SaveAgentActionsError::FlushFailed);
    }
    return {};
  }
};

}  // namespace cse498
