/**
 * @file IOutputManager.hpp
 * @author GitHub Copilot
 **/

#pragma once

#include <memory>
#include <string>
#include <vector>

// #include "../tools/DataLog.hpp"
#include <nlohmann/json.hpp>

#include "IActionLog.hpp"  // For LogLevel definition and ActionEventBase

namespace cse498 {

/**
 * @class IOutputManager
 * @brief Interface for formatting and persisting simulation output.
 */
class IOutputManager {
 public:
  virtual ~IOutputManager() = default;

  /// @brief Sets the output JSON file path (implementation may open lazily on
  /// Flush).
  /// @param path Filesystem path for the JSON log file.
  /// @return true if the path was stored successfully; false if closing a
  /// previous stream failed.
  virtual bool SetOutputFile(const std::string& path) = 0;

  /// @brief Logs a human-readable message with a severity level.
  /// @param level Severity of the message.
  /// @param message Text written to console (if allowed) and buffered in JSON.
  virtual void LogMessage(LogLevel level, const std::string& message) = 0;

  /// @brief Sets the maximum log level that is still emitted.
  /// @param level New threshold; messages with a numerically higher level are
  /// suppressed.
  virtual void SetLogLevel(LogLevel level) noexcept = 0;

  /// @brief Writes the full in-memory JSON (including pending action events) to
  /// disk, replacing the file contents so repeated flushes stay one valid JSON
  /// document.
  /// @return true if the file was written successfully (or no file path is
  /// set).
  virtual bool Flush() = 0;

  /*
  /// @brief Merges simulation entries and statistics into the buffer and
  /// typically flushes.
  /// @param dataLog Entries and aggregate statistics to merge into the buffer.
  virtual void WriteSimulationOutput(const DataLog& dataLog) = 0;*/

  /// @brief Reference to accumulated JSON for tests or inspection; may refresh
  /// cached action_events from pending data (not const for that reason).
  /// @return Reference to the in-memory JSON object (messages, action_events,
  /// etc.).
  virtual const nlohmann::json& GetBufferedLog() noexcept = 0;

  /// @brief Queues action events in memory; disk write happens on Flush().
  /// @param events Action events to append to the pending queue.
  virtual void WriteActionEvents(
      const std::vector<nlohmann::json>& events) = 0;
};

}  // namespace cse498
