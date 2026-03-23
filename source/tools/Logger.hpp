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
  std::string mOutputFilePath;

 public:
  Logger(std::unique_ptr<IActionLog<AgentBase>> actionLog,
         std::unique_ptr<IOutputManager> outputManager,
         const std::string& outputFilePath = "",
         std::unique_ptr<void> replayDriver = nullptr)
      : mActionLog(std::move(actionLog)),
        mOutputManager(std::move(outputManager)),
        mReplayDriver(std::move(replayDriver)),
        mOutputFilePath(outputFilePath) {}

  ~Logger() override = default;

  bool SetOutputFile(const std::string& path) override {
    if (!mOutputManager) return false;
    mOutputFilePath = path;
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

    // todo: send failed events from actionlogto output manager, if we still want to write to console
    return failedEvents;
  }

  bool BeginReplay(const std::string& filePath) override {
    if (!mReplayDriver) {
      return false;
    }

    // todo: Create IReplayDriver interface to replace void* mReplayDriver
    // Currently mReplayDriver is stored as void* which prevents direct method calls.
    // Once IReplayDriver interface exists, we can:
    // 1. Cast mReplayDriver to IReplayDriver*
    // 2. Call ReplayFromFile(filePath) through the interface
    // 3. Properly handle the returned data (if needed or can pass to world directly) without type safety issues
    return true;
  }

  const std::string& GetOutputFilePath() const { 
    return mOutputFilePath; 
  }
};

}  // namespace cse498
