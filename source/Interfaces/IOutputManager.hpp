/**
 * @file IOutputManager.hpp
 * @author GitHub Copilot
 **/

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../tools/DataLog.hpp"
#include "IActionLog.hpp"  // For LogLevel definition and ActionEventBase
#include "nlohmann/json.hpp"

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
  virtual bool SetOutputFile(const std::string& path) = 0;

  /// @brief Logs a human-readable message with a severity level.
  virtual void LogMessage(LogLevel level, const std::string& message) = 0;

  /// @brief Sets the maximum log level that is still emitted.
  virtual void SetLogLevel(LogLevel level) noexcept = 0;

  /// @brief Writes the full in-memory JSON (including pending action events) to
  /// disk.
  virtual bool Flush() = 0;

  /// @brief Merges simulation entries and statistics into the buffer and
  /// typically flushes.
  virtual void WriteSimulationOutput(const DataLog& dataLog) = 0;

  /// @brief Read-only view of the accumulated JSON for tests or inspection.
  virtual const nlohmann::json& GetBufferedLog() const noexcept = 0;

  /// @brief Queues action events in memory; disk write happens on Flush().
  virtual void WriteActionEvents(
      const std::vector<ActionEventBase>& events) = 0;
};

}  // namespace cse498
