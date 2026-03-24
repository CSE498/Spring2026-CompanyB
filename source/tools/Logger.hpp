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

class Logger : public ILogger {
 private:
  std::unique_ptr<IActionLog<AgentBase>> mActionLog;
  std::unique_ptr<IOutputManager> mOutputManager;
  std::unique_ptr<IReplayDriver> mReplayDriver;

 public:
  Logger(std::unique_ptr<IActionLog<AgentBase>> actionLog,
         std::unique_ptr<IOutputManager> outputManager,
         std::unique_ptr<IReplayDriver> replayDriver = nullptr)
      : mActionLog(std::move(actionLog)),
        mOutputManager(std::move(outputManager)),
        mReplayDriver(std::move(replayDriver)) {}

  ~Logger() override = default;

  // I don't think we need this method in the Logger class since the 
  // OutputManager is supposed to generate the output file name 
  // automatically when writing action events.

  // bool SetOutputFile(const std::string& path) override {
  //   if (!mOutputManager) return false;
  //   return mOutputManager->SetOutputFile(path);
  // }

  bool BeginReplay(const std::string& filePath) override {
    if (!mReplayDriver) {
      return false;
    }

    return mReplayDriver->ReplayFromFile(filePath);
  }

  // to implement a method that calls ActionLog's function 
  // and returns a vector of ActionEventBase objects that 
  // can be passed to the Logger's LogActionEvents method 
  // (can make a call to this method inside the func to do).

  /// @brief Log action events from agents and persist to output.
  /// @param events Vector of ActionEventBase objects to be logged.
  /// @return true if successfully logged to output manager.
  bool LogActionEvents(const std::vector<ActionEventBase>& events) {
    if (!mOutputManager) {
      return false;
    }
    mOutputManager->WriteActionEvents(events);
    return true;
  }
};

}  // namespace cse498
