#include "OutputManager.hpp"

#include <filesystem>
#include <iostream>

namespace cse498 {

namespace {
constexpr char kMessagesKey[] = "messages";
constexpr char kEntriesKey[] = "entries";
constexpr char kStatisticsKey[] = "statistics";
}  // namespace

void OutputManager::SetLogLevel(LogLevel level) noexcept { mCurrentLevel = level; }

void OutputManager::ensureOutputStreamOpen() {
  if (mOutputFilePath.empty()) {
    return;
  }
  if (mOutputStream.is_open()) {
    return;
  }

  namespace fs = std::filesystem;
  const fs::path path(mOutputFilePath);
  const fs::path parent = path.parent_path();
  if (!parent.empty()) {
    if (!fs::exists(parent)) {
      std::clog << "[OutputManager] Output directory does not exist: "
                << parent.string() << '\n';
      return;
    }
    if (!fs::is_directory(parent)) {
      std::clog << "[OutputManager] Output path is not a directory: "
                << parent.string() << '\n';
      return;
    }
  }

  mOutputStream.open(mOutputFilePath, std::ios::out | std::ios::trunc);
  if (!mOutputStream.is_open()) {
    std::clog << "[OutputManager] Failed to open output file: " << mOutputFilePath
              << '\n';
  }
}

bool OutputManager::SetOutputFile(const std::string& path) {
  if (mOutputStream.is_open()) {
    mOutputStream.close();
    if (mOutputStream.fail()) {
      return false;
    }
  }
  mOutputFilePath = path;
  return true;
}

void OutputManager::LogMessage(LogLevel level, const std::string &message) {
  // Don't treat "log at Silent" as a valid message
  if (level == LogLevel::Silent) {
    return;
  }
  const int levelValue = static_cast<int>(level);
  const int currentValue = static_cast<int>(mCurrentLevel);
  if (levelValue > currentValue) {
    return;
  }

  std::clog << "[OutputManager] " << message << '\n';

  if (!mOutputFilePath.empty()) {
    ensureOutputStreamOpen();
    if (mOutputStream.is_open()) {
      if (!mBufferedLog.contains(kMessagesKey) ||
          !mBufferedLog[kMessagesKey].is_array()) {
        mBufferedLog[kMessagesKey] = nlohmann::json::array();
      }
      mBufferedLog[kMessagesKey].push_back(
          {{"level", levelValue}, {"text", message}});
    }
  }
}

void OutputManager::WriteSimulationOutput(const DataLog &dataLog) {
  mBufferedLog[kEntriesKey] = nlohmann::json::array();
  for (const auto &entry : dataLog.GetEntries()) {
    mBufferedLog[kEntriesKey].push_back(entry);
  }
  mBufferedLog[kStatisticsKey] = {{"count", dataLog.GetCount()},
                                {"mean", dataLog.GetMean()},
                                {"median", dataLog.GetMedian()},
                                {"min", dataLog.GetMin()},
                                {"max", dataLog.GetMax()}};

  if (!mOutputFilePath.empty()) {
    ensureOutputStreamOpen();
    if (mOutputStream.is_open()) {
      mOutputStream << mBufferedLog.dump(2) << '\n';
      mOutputStream.flush();
    }
  }

  if (mCurrentLevel != LogLevel::Silent) {
    const auto mean = dataLog.GetMean();
    const auto min = dataLog.GetMin();
    const auto max = dataLog.GetMax();
    const auto mean_str =
        mean.has_value() ? std::to_string(mean.value()) : "N/A";
    const auto min_str =
        min.has_value() ? std::to_string(min.value()) : "N/A";
    const auto max_str =
        max.has_value() ? std::to_string(max.value()) : "N/A";
    std::clog << "[Simulation output] entries=" << dataLog.GetCount()
              << " mean=" << mean_str << " min=" << min_str
              << " max=" << max_str << '\n';
  }
}

const nlohmann::json& OutputManager::GetBufferedLog() const noexcept {
  return mBufferedLog;
}

}  // namespace cse498
