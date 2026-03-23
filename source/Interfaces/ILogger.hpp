#pragma once

#include <string>

#include "IActionLog.hpp"
#include "IOutputManager.hpp"

namespace cse498 {

class ILogger {
 public:
  virtual ~ILogger() = default;

  virtual bool SetOutputFile(const std::string& path) = 0;
  virtual bool BeginReplay(const std::string& filePath) = 0;
};

}  // namespace cse498
