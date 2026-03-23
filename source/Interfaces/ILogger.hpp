#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "IActionLog.hpp"
#include "IOutputManager.hpp"

namespace cse498 {

class AgentBase;

class ILogger {
 public:
  virtual ~ILogger() = default;

  virtual bool SetOutputFile(const std::string& path) = 0;
  virtual std::unique_ptr<std::vector<LogEventFailure>> SaveEvents(
      const std::vector<AgentBase*>& agents) = 0;
  virtual bool BeginReplay(const std::string& filePath) = 0;
};

}  // namespace cse498
