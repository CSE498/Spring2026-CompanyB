/**
 * AgentData.hpp
 * @brief Unified agent state DataClasses for all worlds.
 *
 * This file defines the data types for both worlds so any agent or world
 * only needs a single include. TrafficData is used by the traffic
 * world; DiseaseData is used by the infectious-disease world.
 **/

#pragma once

#include <optional>

#include "../tools/StateGridPosition.hpp"
#include "WorldPosition.hpp"

namespace cse498 {

  // ---------------------------------------------------------------------------
// Traffic/swarming world data
// ---------------------------------------------------------------------------
struct TrafficData {
  std::optional<cse498::WorldPosition> destination;
  cse498::WorldPosition position;
  cse498::Direction direction;
  // bool swarm_away;  // true = flee, false = approach
  bool is_active;
  char symbol = '@';
  std::string colour = "blue";
};




// ---------------------------------------------------------------------------
// Infectious-disease world data
// ---------------------------------------------------------------------------

/// Health states for an agent in an infectious-disease simulation.
enum class HealthState { SUSCEPTIBLE, INFECTED, RECOVERED };

/// All per-agent state for the infectious-world simulation.
/// Position, health status, and how long the agent has been in that state
/// are bundled here so they flow through StepWorldBase's DoAction / SetState
/// machinery without a separate health_map.
struct DiseaseData {
  WorldPosition position{};
  HealthState health{HealthState::SUSCEPTIBLE};
  size_t ticks_in_state{0};

  size_t quarantine_ticks{0};
  std::optional<WorldPosition> destination{};
};



}  // namespace cse498
