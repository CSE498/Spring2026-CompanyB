#pragma once

#include <optional>
#include <string>

#include "../core/WorldPosition.hpp"
#include "../tools/StateGridPosition.hpp"

namespace cse498 {
struct TrafficData {
  std::optional<WorldPosition> destination;
  WorldPosition position;
  Direction direction;
  bool is_active{};
  char symbol{};
  std::string colour;  ///< ANSI escape code for display colour.
};
};  // namespace cse498