/**
 * @file AgentData.hpp
 * @brief Unified agent state data classes for step-based worlds.
 *
 * This file defines the lightweight state objects carried by step-based agents.
 * TrafficData is used by the traffic world, and DiseaseData is used by the
 * infectious-disease world.
 **/

#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "../tools/StateGridPosition.hpp"
#include "WorldPosition.hpp"

namespace cse498 {

/**
 * @brief Per-agent state used by traffic/swarming simulations.
 *
 * The swarming agent reads this state to decide whether to move toward a
 * destination, wander, or remain inactive. Worlds update position after
 * consuming movement steps from the agent.
 */
struct TrafficData {
  /// Optional target cell that the agent should approach.
  std::optional<cse498::WorldPosition> destination;

  /// Current grid position.
  cse498::WorldPosition position;

  /// Current facing direction for traffic-style worlds.
  cse498::Direction direction;

  // bool swarm_away;  // true = flee, false = approach

  /// Whether the traffic agent should emit movement steps.
  bool is_active{};

  /// Character used by text or debug renderers.
  char symbol = '@';

  /// Display color name used by visualizations that support it.
  std::string colour = "blue";

  [[nodiscard]] nlohmann::json ToJSON() const {
    nlohmann::json j = nlohmann::json::object();
    if (destination.has_value()) {
      j["destination"] = {{"x", destination->CellX()},
                          {"y", destination->CellY()}};
    }
    j["position"] = {{"x", position.CellX()}, {"y", position.CellY()}};
    j["direction"] = static_cast<int>(direction);
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
        data.destination =
            cse498::WorldPosition{d.at("x").get<int>(), d.at("y").get<int>()};
      }
    }
    if (j.contains("position") && j.at("position").is_object()) {
      auto p = j.at("position");
      if (p.contains("x") && p.contains("y")) {
        data.position =
            cse498::WorldPosition{p.at("x").get<int>(), p.at("y").get<int>()};
      }
    }
    if (j.contains("direction") && j.at("direction").is_number_integer()) {
      int dirValue = j.at("direction").get<int>();
      if (dirValue >= 0 && dirValue <= 3) {
        data.direction = static_cast<cse498::Direction>(dirValue);
      }
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

/**
 * @brief Health state for an agent in an infectious-disease simulation.
 */
enum class HealthState { SUSCEPTIBLE, INFECTED, RECOVERED };

/**
 * @brief Per-agent state for InfectiousWorld.
 *
 * Position, health state, timers, and an optional destination are kept together
 * so the standard StepWorldBase action flow can update the agent without a
 * separate health map.
 */
struct DiseaseData {
  /// Current grid position.
  WorldPosition position{};

  /// Current disease state.
  HealthState health{HealthState::SUSCEPTIBLE};

  /// Number of world ticks spent in the current health state.
  size_t ticks_in_state{0};

  /// Number of infected ticks spent inside a quarantine/treatment zone.
  size_t quarantine_ticks{0};

  /// Optional target cell, such as clinic entrance or recovery exit.
  std::optional<WorldPosition> destination{};

  [[nodiscard]] nlohmann::json ToJSON() const {
    nlohmann::json j = nlohmann::json::object();
    j["position"] = {{"x", position.CellX()}, {"y", position.CellY()}};

    j["susceptible"] = (health == HealthState::SUSCEPTIBLE);
    j["infected"] = (health == HealthState::INFECTED);
    j["recovered"] = (health == HealthState::RECOVERED);

    j["ticks_in_state"] = ticks_in_state;

    j["quarantine_ticks"] = quarantine_ticks;

    if (destination.has_value()) {
      j["destination"] = {{"x", destination->CellX()},
                          {"y", destination->CellY()}};
    }
    return j;
  }

  static DiseaseData FromJSON(const nlohmann::json& j) {
    DiseaseData data{};
    if (j.contains("position") && j.at("position").is_object()) {
      auto p = j.at("position");
      if (p.contains("x") && p.contains("y")) {
        data.position =
            cse498::WorldPosition{p.at("x").get<int>(), p.at("y").get<int>()};
      }
    }

    if (j.contains("susceptible") && j.at("susceptible").is_boolean() &&
        j.at("susceptible").get<bool>()) {
      data.health = HealthState::SUSCEPTIBLE;
    } else if (j.contains("infected") && j.at("infected").is_boolean() &&
               j.at("infected").get<bool>()) {
      data.health = HealthState::INFECTED;
    } else if (j.contains("recovered") && j.at("recovered").is_boolean() &&
               j.at("recovered").get<bool>()) {
      data.health = HealthState::RECOVERED;
    }
    if (j.contains("ticks_in_state") &&
        j.at("ticks_in_state").is_number_unsigned()) {
      data.ticks_in_state = j.at("ticks_in_state").get<size_t>();
    }

    if (j.contains("quarantine_ticks") &&
        j.at("quarantine_ticks").is_number_unsigned()) {
      data.quarantine_ticks = j.at("quarantine_ticks").get<size_t>();
    }

    if (j.contains("destination") && j.at("destination").is_object()) {
      auto d = j.at("destination");
      if (d.contains("x") && d.contains("y")) {
        data.destination =
            cse498::WorldPosition{d.at("x").get<int>(), d.at("y").get<int>()};
      }
    }
    return data;
  }
};

}  // namespace cse498
