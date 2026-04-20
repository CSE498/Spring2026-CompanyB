/**
 * @file AgentData.hpp
 * @brief Unified agent state data classes for step-based worlds.
 *
 * This file defines the lightweight state objects carried by step-based agents.
 * TrafficData is used by the traffic world, and DiseaseData is used by the
 * infectious-disease world.
 **/

#pragma once

#include <optional>

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
  bool is_active;

  /// Character used by text or debug renderers.
  char symbol = '@';

  /// Display color name used by visualizations that support it.
  std::string colour = "blue";
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
};

}  // namespace cse498
