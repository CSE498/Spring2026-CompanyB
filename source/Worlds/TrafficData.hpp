#pragma once

#include "../core/WorldPosition.hpp"
#include "../tools/StateGridPosition.hpp"

namespace cse498 {
struct TrafficData {
  WorldPosition destination;
  WorldPosition position;
  Direction direction;
  bool is_active;
};
};  // namespace cse498