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
    std::string agentId;     ///< Copied from ActionEventBase::agentId.
    std::string actionType;  ///< Copied from ActionEventBase::actionType.
    LogLevel logLevel;       ///< Copied from ActionEventBase::logLevel.
    uint64_t timestamp;      ///< Copied from ActionEventBase::timestamp.
  };

  std::string mOutputFilePath{};  ///< Target JSON path; empty skips file I/O.
  LogLevel mCurrentLevel = LogLevel::Normal;  ///< Threshold for ShouldLog.
  std::ofstream mOutputStream;  ///< Opened around Flush when writing to disk.
  nlohmann::json mBufferedLog =
      nlohmann::json::object();  ///< Serialized on Flush.
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
  /// @param messageLevel Severity of the candidate message.
  /// @return true if the message should be logged at the current threshold.
  [[nodiscard]] bool ShouldLog(LogLevel messageLevel) const noexcept;

  /// @brief Writes a single line to the console (stderr via std::clog).
  /// @param message Line of text (without trailing newline).
  void WriteConsole(const std::string& message);

  /// @brief Appends a message object to the in-memory JSON under the "messages"
  /// key.
  /// @param level Severity stored in the JSON entry.
  /// @param message Text stored in the JSON entry.
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
  /// @param path Filesystem path for the JSON log file.
  /// @return true if the path was stored (see base for stream-close failure).
  bool SetOutputFile(const std::string& path) override;

  /// @brief If allowed by the current log level, prints to console and buffers
  /// under "messages".
  /// @param level Severity of the message.
  /// @param message Text to emit and buffer.
  void LogMessage(LogLevel level, const std::string& message) override;

  /// @brief Updates the filter used by ShouldLog for subsequent messages.
  void SetLogLevel(LogLevel level) noexcept override;

  /// @brief Serializes the full JSON buffer (including action events) to the
  /// output file, then closes the stream so each flush overwrites a single JSON
  /// document.
  /// @return true if the file was written successfully (or no path is set).
  bool Flush() override;

  /*/// @copydoc IOutputManager::WriteSimulationOutput
  void WriteSimulationOutput(const DataLog& dataLog) override;*/

  /// @brief Returns the in-memory JSON; rebuilds "action_events" from the
  /// pending vector first.
  /// @return Reference to mBufferedLog (includes pending action events).
  [[nodiscard]] const nlohmann::json& GetBufferedLog() noexcept override;

  /// @brief Appends events to the pending vector; no file I/O until Flush().
  /// @param events Events to queue for serialization on the next flush.
  void WriteActionEvents(const std::vector<ActionEventBase>& events) override;
};

}  // namespace cse498
