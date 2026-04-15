#pragma once

#include <optional>
#include "WorldPosition.hpp"
#include "../tools/StateGridPosition.hpp"

namespace cse498 {

struct TrafficData {
  std::optional<cse498::WorldPosition> destination;
  cse498::WorldPosition position;
  cse498::Direction direction;
  //bool swarm_away;  // true = flee, false = approach
  bool is_active;
  char symbol = '@';
  std::string colour; 
};

// Still needs more work, gotta figure out how we want SwarmingAgents to act in
// the infectiious world, but this is a start

enum class HealthState { SUSCEPTIBLE, INFECTED, RECOVERED };

struct DiseaseData {
  double infection_probability;
  cse498::HealthState infection_state;
  std::optional<cse498::WorldPosition> destination;
  cse498::WorldPosition position;
  // cse498::Direction direction;
};

}  // namespace cse498