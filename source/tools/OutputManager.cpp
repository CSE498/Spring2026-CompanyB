#include "OutputManager.hpp"

#include <filesystem>
#include <iostream>
#include <utility>

#include "DataLog.hpp"

namespace cse498 {

namespace {
constexpr char kMessagesKey[] = "messages";
constexpr char kEntriesKey[] = "entries";
constexpr char kStatisticsKey[] = "statistics";
}  // namespace

void OutputManager::SetLogLevel(LogLevel level) noexcept { mCurrentLevel = level; }

OutputManager::OutputManager(std::string outputFilePath, LogLevel level)
    : mOutputFilePath(std::move(outputFilePath)), mCurrentLevel(level) {
  openOutputStream();
}

OutputManager::~OutputManager() {
  if (mOutputStream.is_open()) {
    mOutputStream.close();
  }
}

bool OutputManager::openOutputStream() {
  if (mOutputFilePath.empty()) {
    return true;
  }
  if (mOutputStream.is_open()) {
    return true;
  }

  namespace fs = std::filesystem;
  const fs::path path(mOutputFilePath);
  const fs::path parent = path.parent_path();
  if (!parent.empty()) {
    if (!fs::exists(parent)) {
      std::clog << "[OutputManager] Output directory does not exist: "
                << parent.string() << '\n';
      return false;
    }
    if (!fs::is_directory(parent)) {
      std::clog << "[OutputManager] Output path is not a directory: "
                << parent.string() << '\n';
      return false;
    }
  }

  mOutputStream.open(mOutputFilePath, std::ios::out | std::ios::trunc);
  if (!mOutputStream.is_open()) {
    std::clog << "[OutputManager] Failed to open output file: " << mOutputFilePath
              << '\n';
    return false;
  }
  return true;
}

bool OutputManager::SetOutputFile(const std::string& path) {
  if (mOutputStream.is_open()) {
    mOutputStream.close();
    if (mOutputStream.fail()) {
      return false;
    }
  }
  mOutputFilePath = path;
  return openOutputStream();
}

void OutputManager::LogMessage(LogLevel level, const std::string &message) {
  LogEntry(kMessagesKey, level, message);
}

void OutputManager::LogConsole(LogLevel level, const std::string& message) {
  const int levelValue = static_cast<int>(level);
  if (level == LogLevel::Silent ||
      levelValue > static_cast<int>(mCurrentLevel)) {
    return;
  }
  std::clog << "[OutputManager] " << message << '\n';
}

void OutputManager::LogFile(LogLevel level, const std::string& message) {
  const int levelValue = static_cast<int>(level);
  if (level == LogLevel::Silent ||
      levelValue > static_cast<int>(mCurrentLevel)) {
    return;
  }
  if (mOutputStream.is_open()) {
    if (!mBufferedLog.contains(kMessagesKey) ||
        !mBufferedLog[kMessagesKey].is_array()) {
      mBufferedLog[kMessagesKey] = nlohmann::json::array();
    }
    mBufferedLog[kMessagesKey].push_back(
        {{"level", levelValue}, {"text", message}});
  }
}

bool OutputManager::Flush() {
  if (!mOutputStream.is_open()) {
    return mOutputFilePath.empty();
  }
  mOutputStream << mBufferedLog.dump(2) << '\n';
  mOutputStream.flush();
  return !mOutputStream.fail();
}

void OutputManager::LogMessage(const std::string& message) {
  LogMessage(LogLevel::Normal, message);
}

void OutputManager::LogEntry(const std::string& category,
                             LogLevel level,
                             const std::string& message) {
  const int levelValue = static_cast<int>(level);
  if (level == LogLevel::Silent ||
      levelValue > static_cast<int>(mCurrentLevel)) {
    return;
  }

  LogConsole(level, message);

  if (!mOutputStream.is_open()) {
    return;
  }
  if (!mBufferedLog.contains(category) || !mBufferedLog[category].is_array()) {
    mBufferedLog[category] = nlohmann::json::array();
  }
  mBufferedLog[category].push_back({{"level", levelValue}, {"text", message}});
}

void OutputManager::WriteSimulationOutput(const DataLog &dataLog) {
  mBufferedLog[kEntriesKey] = nlohmann::json::array();
  for (const auto &entry : dataLog.GetEntries()) {
    mBufferedLog[kEntriesKey].push_back(entry);
  }
  mBufferedLog[kStatisticsKey] = nlohmann::json::object(
      {{"count", dataLog.GetCount()},
       {"mean", dataLog.GetMean().value_or(0.0)},
       {"median", dataLog.GetMedian().value_or(0.0)},
       {"min", dataLog.GetMin().value_or(0.0)},
       {"max", dataLog.GetMax().value_or(0.0)}});

  Flush();

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
