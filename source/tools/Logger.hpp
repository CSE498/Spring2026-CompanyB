#pragma once

#include <memory>
#include <string>

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
  std::string mOutputFilePath;

 public:
  Logger(std::unique_ptr<IActionLog<AgentBase>> actionLog,
         std::unique_ptr<IOutputManager> outputManager,
         std::unique_ptr<IReplayDriver> replayDriver = nullptr,
         const std::string& outputFilePath = "")
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

  bool BeginReplay(const std::string& filePath) override {
    if (!mReplayDriver) {
      return false;
    }

    return mReplayDriver->ReplayFromFile(filePath, mOutputFilePath);
  }

  const std::string& GetOutputFilePath() const { return mOutputFilePath; }
};

}  // namespace cse498
