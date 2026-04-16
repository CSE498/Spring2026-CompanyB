#include "OutputManager.hpp"

#include <filesystem>
#include <iostream>

#include "DataLog.hpp"

namespace cse498 {

namespace {
constexpr char kMessagesKey[] = "messages";
constexpr char kEntriesKey[] = "entries";
constexpr char kStatisticsKey[] = "statistics";
constexpr char kActionEventsKey[] = "action_events";

/// Spaces per nesting level for pretty-printed JSON written to the log file
/// (nlohmann::json::dump indent argument).
constexpr int JsonLogIndentWidth = 2;

/// @brief Default relative path for the JSON log when no path is set
/// explicitly.
std::string MakeDefaultLogFilePath() {
  namespace fs = std::filesystem;
  const fs::path dir = fs::current_path() / "logs";
  return (dir / "simulation_log.json").string();
}
}  // namespace

void OutputManager::SetLogLevel(LogLevel level) noexcept {
  mCurrentLevel = level;
}

OutputManager::OutputManager(LogLevel level)
    : mOutputFilePath(MakeDefaultLogFilePath()), mCurrentLevel(level) {}

/// @brief Closes the file stream if it was opened.
OutputManager::~OutputManager() {
  if (mOutputStream.is_open()) {
    mOutputStream.close();
  }
}

bool OutputManager::OpenOutputStream() {
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
      std::error_code ec;
      fs::create_directories(parent, ec);
      if (ec) {
        std::clog << "[OutputManager] Failed to create output directory: "
                  << parent.string() << '\n';
        return false;
      }
    }
    if (!fs::is_directory(parent)) {
      std::clog << "[OutputManager] Output path is not a directory: "
                << parent.string() << '\n';
      return false;
    }
  }

  mOutputStream.open(mOutputFilePath, std::ios::out | std::ios::trunc);
  if (!mOutputStream.is_open()) {
    std::clog << "[OutputManager] Failed to open output file: "
              << mOutputFilePath << '\n';
    return false;
  }
  return true;
}

bool OutputManager::ShouldLog(LogLevel messageLevel) const noexcept {
  if (messageLevel == LogLevel::Silent) {
    return false;
  }
  const int levelValue = static_cast<int>(messageLevel);
  return levelValue <= static_cast<int>(mCurrentLevel);
}

void OutputManager::WriteConsole(const std::string& message) {
  std::clog << "[OutputManager] " << message << '\n';
}

void OutputManager::BufferMessage(LogLevel level, const std::string& message) {
  const int levelValue = static_cast<int>(level);
  if (!mBufferedLog.contains(kMessagesKey) ||
      !mBufferedLog[kMessagesKey].is_array()) {
    mBufferedLog[kMessagesKey] = nlohmann::json::array();
  }
  mBufferedLog[kMessagesKey].push_back(
      {{"level", levelValue}, {"text", message}});
}

void OutputManager::LogMessage(LogLevel level, const std::string& message) {
  if (!ShouldLog(level)) {
    return;
  }
  WriteConsole(message);
  BufferMessage(level, message);
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

void OutputManager::RebuildActionEventsJson() {
  mBufferedLog[kActionEventsKey] = nlohmann::json::array();
  for (const auto& r : mPendingActionEvents) {
    mBufferedLog[kActionEventsKey].push_back(r);
  }
}

bool OutputManager::Flush() {
  RebuildActionEventsJson();
  if (mOutputFilePath.empty()) {
    return true;
  }
  if (!mOutputStream.is_open() && !OpenOutputStream()) {
    return false;
  }
  mOutputStream << mBufferedLog.dump(JsonLogIndentWidth) << '\n';
  mOutputStream.flush();
  const bool ok = !mOutputStream.fail();
  // Close so the next Flush() does not skip OpenOutputStream()'s trunc open;
  // otherwise multiple dumps would concatenate and break JSON consumers.
  mOutputStream.close();
  if (ok) {
    std::clog << "log saved in " << mOutputFilePath << '\n';
  }
  return ok;
}

/*
void OutputManager::WriteSimulationOutput(const DataLog& dataLog) {
  mBufferedLog[kEntriesKey] = nlohmann::json::array();
  for (const auto& entry : dataLog.GetEntries()) {
    mBufferedLog[kEntriesKey].push_back(entry);
  }
  mBufferedLog[kStatisticsKey] =
      nlohmann::json::object({{"count", dataLog.GetCount()},
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
    const auto min_str = min.has_value() ? std::to_string(min.value()) : "N/A";
    const auto max_str = max.has_value() ? std::to_string(max.value()) : "N/A";
    std::clog << "[Simulation output] entries=" << dataLog.GetCount()
              << " mean=" << mean_str << " min=" << min_str
              << " max=" << max_str << '\n';
  }
}*/

const nlohmann::json& OutputManager::GetBufferedLog() noexcept {
  RebuildActionEventsJson();
  return mBufferedLog;
}

void OutputManager::WriteActionEvents(
    const std::vector<nlohmann::json>& events) {
  for (const auto& e : events) {
    mPendingActionEvents.push_back(e);
  }
}

}  // namespace cse498
