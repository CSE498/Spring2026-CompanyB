#pragma once

#include <string>

namespace cse498 {

class IReplayDriver {
 public:
  virtual ~IReplayDriver() = default;

  virtual bool ReplayFromFile(const std::string& filePath,
                              const std::string& outputFilePath) = 0;
};

}  // namespace cse498
