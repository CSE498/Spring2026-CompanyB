#pragma once

#include <string>

#include "../core/WorldPosition.hpp"
#include "../tools/StateGridPosition.hpp"

#include "nlohmann/json.hpp"

namespace cse498 {
struct TrafficData {
  WorldPosition destination;
  WorldPosition position;
  Direction direction;
  bool is_active;
  char symbol;
  std::string colour;  ///< ANSI escape code for display colour.

  nlohmann::json ToJson() const {
    return nlohmann::json{
        {"destination", destination.ToJson()},
        {"position", position.ToJson()},
        {"direction", direction},
        {"is_active", is_active},
        {"symbol", symbol},
        {"colour", colour},
    };
  }

  TrafficData(const nlohmann::json& j) {
    destination = WorldPosition(j.at("destination"));
    position = WorldPosition(j.at("position"));
    direction = j.at("direction").get<Direction>();
    is_active = j.at("is_active").get<bool>();
    symbol = j.at("symbol").get<char>();
    colour = j.at("colour").get<std::string>();
  }
};
};  // namespace cse498