#pragma once

#include <optional>
#include <string>
#include <nlohmann/json.hpp>

#include "../tools/StateGridPosition.hpp"
#include "WorldPosition.hpp"

namespace cse498 {

struct TrafficData {
  std::optional<cse498::WorldPosition> destination;
  cse498::WorldPosition position;
  cse498::Direction direction;
  // bool swarm_away;  // true = flee, false = approach
  bool is_active{};
  char symbol = '@';
  std::string colour = "blue";

  [[nodiscard]] nlohmann::json ToJSON() const {
    nlohmann::json j = nlohmann::json::object();
    if (destination.has_value()) {
      j["destination"] = {{"x", destination->CellX()}, {"y", destination->CellY()}};
    }
    j["position"] = {{"x", position.CellX()}, {"y", position.CellY()}};
    
    switch (direction) {
      case cse498::Direction::North: j["direction"] = "North"; break;
      case cse498::Direction::South: j["direction"] = "South"; break;
      case cse498::Direction::West:  j["direction"] = "West"; break;
      case cse498::Direction::East:  j["direction"] = "East"; break;
      default: j["direction"] = "North"; break;
    }
    
    j["is_active"] = is_active;
    j["symbol"] = std::string(1, symbol);
    j["colour"] = colour;
    return j;
  }

  static TrafficData FromJSON(const nlohmann::json& j) {
    TrafficData data{};
    if (j.contains("destination") && j.at("destination").is_object()) {
      auto d = j.at("destination");
      if (d.contains("x") && d.contains("y")) {
        data.destination = cse498::WorldPosition{d.at("x").get<int>(), d.at("y").get<int>()};
      }
    }
    if (j.contains("position") && j.at("position").is_object()) {
      auto p = j.at("position");
      if (p.contains("x") && p.contains("y")) {
        data.position = cse498::WorldPosition{p.at("x").get<int>(), p.at("y").get<int>()};
      }
    }
    if (j.contains("direction") && j.at("direction").is_string()) {
      std::string dir = j.at("direction").get<std::string>();
      if (dir == "North") data.direction = cse498::Direction::North;
      else if (dir == "South") data.direction = cse498::Direction::South;
      else if (dir == "West")  data.direction = cse498::Direction::West;
      else if (dir == "East")  data.direction = cse498::Direction::East;
    }
    if (j.contains("is_active") && j.at("is_active").is_boolean()) {
      data.is_active = j.at("is_active").get<bool>();
    }
    if (j.contains("symbol") && j.at("symbol").is_string()) {
      std::string s = j.at("symbol").get<std::string>();
      if (!s.empty()) data.symbol = s[0];
    }
    if (j.contains("colour") && j.at("colour").is_string()) {
      data.colour = j.at("colour").get<std::string>();
    }
    return data;
  }
};

// Still needs more work, gotta figure out how we want SwarmingAgents to act in
// the infectiious world, but this is a start

enum class HealthState { SUSCEPTIBLE, INFECTED, RECOVERED };

struct DiseaseData {
  double infection_probability{};
  cse498::HealthState infection_state;
  std::optional<cse498::WorldPosition> destination;
  cse498::WorldPosition position;
  // cse498::Direction direction;

  [[nodiscard]] nlohmann::json ToJSON() const {
    nlohmann::json j = nlohmann::json::object();
    j["infection_probability"] = infection_probability;
    
    switch(infection_state) {
        case HealthState::SUSCEPTIBLE: j["infection_state"] = "susceptible"; break;
        case HealthState::INFECTED:    j["infection_state"] = "infected"; break;
        case HealthState::RECOVERED:   j["infection_state"] = "recovered"; break;
        default:                       j["infection_state"] = "susceptible"; break;
    }
    
    if (destination.has_value()) {
        j["destination"] = {{"x", destination->CellX()}, {"y", destination->CellY()}};
    }
    j["position"] = {{"x", position.CellX()}, {"y", position.CellY()}};
    return j;
  }

  static DiseaseData FromJSON(const nlohmann::json& j) {
      DiseaseData data{};
      if (j.contains("infection_probability") && j.at("infection_probability").is_number()) {
          data.infection_probability = j.at("infection_probability").get<double>();
      }
      if (j.contains("infection_state") && j.at("infection_state").is_string()) {
          std::string hs = j.at("infection_state").get<std::string>();
          if (hs == "susceptible") data.infection_state = HealthState::SUSCEPTIBLE;
          else if (hs == "infected") data.infection_state = HealthState::INFECTED;
          else if (hs == "recovered") data.infection_state = HealthState::RECOVERED;
      }
      if (j.contains("destination") && j.at("destination").is_object()) {
          auto d = j.at("destination");
          if (d.contains("x") && d.contains("y")) {
              data.destination = cse498::WorldPosition{d.at("x").get<int>(), d.at("y").get<int>()};
          }
      }
      if (j.contains("position") && j.at("position").is_object()) {
          auto p = j.at("position");
          if (p.contains("x") && p.contains("y")) {
              data.position = cse498::WorldPosition{p.at("x").get<int>(), p.at("y").get<int>()};
          }
      }
      return data;
  }
};

}  // namespace cse498