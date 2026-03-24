/**
 * This file is the interface for formatting and persisting simulation output.
 * A simple logging system for programmers to log events and for the Data team
 * to write finalized log data and statistics to file and console.
 * @brief API class for writing logs and statistics to JSON and console with
 *configurable log levels.
 * @note Status: INITIAL DESIGN
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
  /// @brief Current path of the JSON log file to be written.
  std::string mOutputFilePath{};

  /// @brief Active log level controlling which messages are emitted.
  LogLevel mCurrentLevel = LogLevel::Normal;

  /// @brief File stream used to write serialized log data and summaries.
  std::ofstream mOutputStream;

  /// @brief In-memory JSON that accumulates simulation outputs and insights.
  nlohmann::json mBufferedLog = nlohmann::json::object();

  /// @brief Opens the configured output file if possible.
  /// @return true when no file path is configured or stream opens successfully.
  bool openOutputStream();

 public:
  OutputManager() = default;
  explicit OutputManager(std::string outputFilePath,
                         LogLevel level = LogLevel::Normal);
  ~OutputManager() override;

  OutputManager(const OutputManager&) = delete;
  OutputManager& operator=(const OutputManager&) = delete;

  /// @brief Sets the output file path and closes any previously open file.
  /// @param path Path for the JSON log file.
  /// @return true on success, false if closing the previous file failed.
  bool SetOutputFile(const std::string& path) override;

  /// @brief Records a log message at the given level; writes to file and/or
  /// console per current log level.
  /// @param level Log level of this message.
  /// @param message Message text.
  void LogMessage(LogLevel level, const std::string& message) override;

  /// @brief Records a message with default Normal level.
  /// @param message Message text.
  void LogMessage(const std::string& message);

  /// @brief Records a category-tagged entry with standard level filtering.
  /// @param category Top-level category key for persistence.
  /// @param level Log level of this message.
  /// @param message Message text.
  void LogEntry(const std::string& category, LogLevel level,
                const std::string& message) override;

  /// @brief Records a message to console only.
  /// @param level Log level of this message.
  /// @param message Message text.
  void LogConsole(LogLevel level, const std::string& message);

  /// @brief Records a message to file buffer only.
  /// @param level Log level of this message.
  /// @param message Message text.
  void LogFile(LogLevel level, const std::string& message);

  /// @brief Sets the active log level (e.g. at start of a simulation run).
  /// @param level New log level.
  void SetLogLevel(LogLevel level) noexcept override;

  /// @brief Flushes current buffered JSON to the configured output file.
  /// @return true when flushed or no output file configured, false on write
  /// failure.
  bool Flush() override;

  /// @brief Writes the complete log and computed statistics from DataLog to
  /// JSON file and console. Called by DataLog::WriteToOutput() at the end of a
  /// simulation run.
  /// @param dataLog DataLog instance containing entries and statistics.
  void WriteSimulationOutput(const DataLog& dataLog);

  /// @brief Returns the in-memory buffered log (for unit testing).
  /// @return Const reference to the accumulated JSON (messages, entries,
  /// statistics).
  [[nodiscard]] const nlohmann::json& GetBufferedLog() const noexcept;

  /// @brief Writes action events to output file with auto-generated file naming.
  /// @param events Vector of ActionEventBase objects to be persisted.
  void WriteActionEvents(const std::vector<ActionEventBase>& events) override;
};

}  // namespace cse498
