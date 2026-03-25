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

  virtual bool SetOutputFile(const std::string& path) = 0;
  virtual void LogMessage(LogLevel level, const std::string& message) = 0;
  virtual void LogEntry(const std::string& category, LogLevel level,
                        const std::string& message) = 0;
  virtual void SetLogLevel(LogLevel level) noexcept = 0;
  virtual bool Flush() = 0;
  virtual void WriteSimulationOutput(const DataLog& dataLog) = 0;
  virtual const nlohmann::json& GetBufferedLog() const noexcept = 0;
  /// @brief Writes action events to output, with auto-generated file naming.
  /// @param events Vector of ActionEventBase objects representing agent
  /// actions.
  virtual void WriteActionEvents(
      const std::vector<ActionEventBase>& events) = 0;
};

}  // namespace cse498
