/**
 * @brief Formats and persists simulation output: console + JSON buffer; file
 * I/O on Flush().
 **/

#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "../Interfaces/IActionLog.hpp"
#include "../Interfaces/IOutputManager.hpp"
#include "nlohmann/json.hpp"

namespace cse498 {
class DataLog;

/// @brief Formats and persists simulation log data and programmer log messages.
class OutputManager : public IOutputManager {
 private:
  /// @brief Owned copy of an action event for deferred persistence
  /// (string_view-safe).
  struct ActionEventRecord {
    std::string agentId;
    std::string actionType;
    LogLevel logLevel;
    uint64_t timestamp;
  };

  std::string mOutputFilePath{};
  LogLevel mCurrentLevel = LogLevel::Normal;
  std::ofstream mOutputStream;
  nlohmann::json mBufferedLog = nlohmann::json::object();
  /// @brief Queued action events; serialized to JSON on Flush() /
  /// GetBufferedLog().
  std::vector<ActionEventRecord> mPendingActionEvents;

  /// @brief Creates parent directories if needed and opens the output file for
  /// writing.
  /// @return false if the path cannot be opened or the parent is not a
  /// directory.
  bool OpenOutputStream();

  /// @brief Returns whether a message at @p messageLevel should be emitted at
  /// the current log level.
  [[nodiscard]] bool ShouldLog(LogLevel messageLevel) const noexcept;

  /// @brief Writes a single line to the console (stderr via std::clog).
  void WriteConsole(const std::string& message);

  /// @brief Appends a message object to the in-memory JSON under the "messages"
  /// key.
  void BufferMessage(LogLevel level, const std::string& message);

  /// @brief Rebuilds the "action_events" array in @c mBufferedLog from @c
  /// mPendingActionEvents.
  void RebuildActionEventsJson();

 public:
  /// @brief Constructs with default output path @c logs/simulation_log.json (no
  /// disk write until Flush).
  explicit OutputManager(LogLevel level = LogLevel::Normal);

  /// @brief Closes the output stream if open.
  ~OutputManager() override;

  OutputManager(const OutputManager&) = delete;
  OutputManager& operator=(const OutputManager&) = delete;

  /// @brief Sets the output file path; does not open the file until the next
  /// Flush().
  bool SetOutputFile(const std::string& path) override;

  /// @brief If allowed by the current log level, prints to console and buffers
  /// under "messages".
  void LogMessage(LogLevel level, const std::string& message) override;

  /// @brief Updates the filter used by ShouldLog for subsequent messages.
  void SetLogLevel(LogLevel level) noexcept override;

  /// @brief Serializes the full JSON buffer (including action events) to the
  /// output file, then closes the stream so each flush overwrites a single JSON
  /// document.
  bool Flush() override;

  /// @brief Writes DataLog entries and statistics, then calls Flush().
  void WriteSimulationOutput(const DataLog& dataLog) override;

  /// @brief Returns the in-memory JSON; rebuilds "action_events" from the
  /// pending vector first.
  [[nodiscard]] const nlohmann::json& GetBufferedLog() noexcept override;

  /// @brief Appends events to the pending vector; no file I/O until Flush().
  void WriteActionEvents(const std::vector<ActionEventBase>& events) override;
};

}  // namespace cse498
