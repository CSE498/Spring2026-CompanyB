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

#include "../Interfaces/IOutputManager.hpp"
#include "DataLog.hpp"
#include "nlohmann/json.hpp"

namespace cse498 {

/// @brief Formats and persists simulation log data and programmer log messages.
class OutputManager : public IOutputManager {

 private:
  /// @brief Current path of the JSON log file to be written.
  std::string mOutputFilePath;

  /// @brief Active log level controlling which messages are emitted.
  LogLevel mCurrentLevel = LogLevel::Normal;

  /// @brief File stream used to write serialized log data and summaries.
  std::ofstream mOutputStream;

  /// @brief In-memory JSON that accumulates simulation outputs and insights.
  nlohmann::json mBufferedLog = nlohmann::json::object();

  /// @brief Ensures the output file is open for writing; no-op if path empty or
  /// already open.
  void ensureOutputStreamOpen();

 public:
  OutputManager() = default;
  ~OutputManager() = default;

  OutputManager(const OutputManager&) = delete;
  OutputManager& operator=(const OutputManager&) = delete;

  /// @brief Sets the output file path and closes any previously open file.
  /// @param path Path for the JSON log file.
  /// @return true on success, false if closing the previous file failed.
  bool SetOutputFile(const std::string& path);

  /// @brief Records a log message at the given level; writes to file and/or
  /// console per current log level.
  /// @param level Log level of this message.
  /// @param message Message text.
  void LogMessage(LogLevel level, const std::string& message);

  /// @brief Sets the active log level (e.g. at start of a simulation run).
  /// @param level New log level.
  void SetLogLevel(LogLevel level) noexcept;

  /// @brief Writes the complete log and computed statistics from DataLog to
  /// JSON file and console. Called by DataLog::WriteToOutput() at the end of a
  /// simulation run.
  /// @param dataLog DataLog instance containing entries and statistics.
  void WriteSimulationOutput(const DataLog& dataLog);

  /// @brief Returns the in-memory buffered log (for unit testing).
  /// @return Const reference to the accumulated JSON (messages, entries,
  /// statistics).
  [[nodiscard]] const nlohmann::json& GetBufferedLog() const noexcept;
};

}  // namespace cse498
