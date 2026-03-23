#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../Interfaces/IActionLog.hpp"
#include "../Interfaces/ILogger.hpp"
#include "../Interfaces/IOutputManager.hpp"
#include "ReplayDriver.hpp"

namespace cse498 {

class AgentBase;

class Logger : public ILogger {
 private:
  std::unique_ptr<IActionLog<AgentBase>> mActionLog;
  std::unique_ptr<IOutputManager> mOutputManager;
  std::unique_ptr<void> mReplayDriver;

 public:
  Logger(std::unique_ptr<IActionLog<AgentBase>> actionLog,
         std::unique_ptr<IOutputManager> outputManager,
         std::unique_ptr<void> replayDriver = nullptr)
      : mActionLog(std::move(actionLog)),
        mOutputManager(std::move(outputManager)),
        mReplayDriver(std::move(replayDriver)) {}

  ~Logger() override = default;

  bool SetOutputFile(const std::string& path) override {
    if (!mOutputManager) return false;
    return mOutputManager->SetOutputFile(path);
  }

  std::unique_ptr<std::vector<LogEventFailure>> SaveEvents(
      const std::vector<AgentBase*>& agents) override {
    if (!mActionLog || !mOutputManager) {
      return nullptr;
    }

    std::vector<AgentBase> agentCopies;
    for (const auto* agentPtr : agents) {
      if (agentPtr) {
        agentCopies.push_back(*agentPtr);
      }
    }

    auto failedEvents = mActionLog->LogAgentActions(agentCopies);

    // TODO: Persist validated events via IOutputManager, if we still want to write to console
    return failedEvents;
  }

  bool BeginReplay(const std::string& filePath) override {
    if (!mReplayDriver) {
      return false;
    }

    // TODO: Implement replay loading logic, filling up container with data from file
    return true;
  }
};

}  // namespace cse498
