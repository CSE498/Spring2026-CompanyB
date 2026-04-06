/**
 * @file SwarmingAgent.hpp
 * @brief A simple agent that tries to swarm around to a target
 * area by talking to other agents
 */

#pragma once

#include <cmath>
#include <limits>
#include <random>
#include <string>

#include "../core/ActionNames.hpp"
#include "../core/AgentBase.hpp"
#include "../core/WorldPosition.hpp"
#include "../tools/RobinHoodMap.hpp"

namespace cse498 {

/**
 * @brief A simple agent that tries to swarm around to a target area
 * by talking to other agents
 *
 * This agent maintains a map of known locations items or other points of
 * interest and their positions. It shares this knowledge with nearby agents and
 * uses it to navigate towards a specified target.
 */
class SwarmingAgent : public AgentBase {
 protected:
  /// @brief  ID of the Agents target area (item?)
  size_t target_id = SIZE_MAX;

  /// @brief where the target is (unknown until found)
  WorldPosition target_pos;

  /// @brief map of the locations we know about, may contain target
  RobinHoodMap<size_t, WorldPosition> known_locations;

  /// @brief random number generator for any random decisions this agents makes
  std::mt19937 rng;

  struct MoveIntent {
    double dx = 0.0;
    double dy = 0.0;
  };

  void MergeKnowledgeFromNeighbor();
  MoveIntent ChooseTargetIntent();
  MoveIntent ChooseRandomIntent();
  size_t IntentToAction(const MoveIntent& intent);

 public:
  SwarmingAgent(size_t id, const std::string& name, const WorldBase& world)
      : AgentBase(id, name, world), rng(static_cast<unsigned>(id)) {}

  ~SwarmingAgent() override = default;

  bool Initialize() override {
    return HasAction(ActionNames::UP) && HasAction(ActionNames::DOWN) &&
           HasAction(ActionNames::LEFT) && HasAction(ActionNames::RIGHT);
  }

  /// Set the ID of the target location this agent is seeking.
  SwarmingAgent& SetTarget(const size_t& id) {
    target_id = id;
    return *this;
  }

  /// Seed this agent's knowledge with a known location.
  SwarmingAgent& AddKnownLocation(const size_t& name, WorldPosition pos) {
    known_locations.insert(name, pos);
    return *this;
  }

  /// Get the map of all locations this agent knows about (for knowledge
  /// sharing).
  [[nodiscard]] const RobinHoodMap<size_t, WorldPosition>& GetKnownLocations()
      const {
    return known_locations;
  }

  [[nodiscard]] size_t SelectAction(const WorldGrid& grid) override;
};

}  // namespace cse498
