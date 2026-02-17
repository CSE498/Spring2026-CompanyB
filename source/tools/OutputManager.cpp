#include "OutputManager.hpp"

#include <iostream>

namespace cse498 {

void OutputManager::SetLogLevel(LogLevel level) { mCurrentLevel = level; }

void OutputManager::ensureOutputStreamOpen() {
  if (mOutputFilePath.empty()) {
    return;
  }
  if (!mOutputStream.is_open()) {
    mOutputStream.open(mOutputFilePath, std::ios::out | std::ios::trunc);
  }
}

void OutputManager::SetOutputFile(const std::string &path) {
  if (mOutputStream.is_open()) {
    mOutputStream.close();
  }
  mOutputFilePath = path;
}

void OutputManager::LogMessage(LogLevel level, const std::string &message) {
  // Silent: no output
  if (mCurrentLevel == LogLevel::Silent) {
    return;
  }
  // Only emit if message level is at or above current verbosity
  const int levelValue = static_cast<int>(level);
  const int currentValue = static_cast<int>(mCurrentLevel);
  if (levelValue < currentValue) {
    return;
  }

  const bool toConsole = (mCurrentLevel != LogLevel::Silent);
  const bool toFile =
      (mCurrentLevel != LogLevel::Silent && !mOutputFilePath.empty());

  if (toConsole) {
    std::clog << "[OutputManager] " << message << std::endl;
  }
  if (toFile) {
    ensureOutputStreamOpen();
    if (mOutputStream.is_open()) {
      mBufferedLog["messages"] =
          mBufferedLog.value("messages", nlohmann::json::array());
      mBufferedLog["messages"].push_back(
          {{"level", levelValue}, {"text", message}});
    }
  }
}

void OutputManager::WriteSimulationOutput(const DataLog &dataLog) {
  mBufferedLog["entries"] = nlohmann::json::array();
  for (const auto &entry : dataLog.GetEntries()) {
    mBufferedLog["entries"].push_back(entry);
  }
  mBufferedLog["statistics"] = {{"count", dataLog.GetCount()},
                                {"mean", dataLog.GetMean()},
                                {"median", dataLog.GetMedian()},
                                {"min", dataLog.GetMin()},
                                {"max", dataLog.GetMax()}};

  if (!mOutputFilePath.empty()) {
    ensureOutputStreamOpen();
    if (mOutputStream.is_open()) {
      mOutputStream << mBufferedLog.dump(2) << std::endl;
    }
  }

  if (mCurrentLevel != LogLevel::Silent) {
    std::clog << "[Simulation output] entries=" << dataLog.GetCount()
              << " mean=" << dataLog.GetMean() << " min=" << dataLog.GetMin()
              << " max=" << dataLog.GetMax() << std::endl;
  }
}

}  // namespace cse498
