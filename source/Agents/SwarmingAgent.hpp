/**
 * @file SwarmingAgent.hpp
 * @brief A simple agent that tries to swarm around to a target
 * area by talking to other agents
 */

#pragma once

#include "../core/AgentBase.hpp"
#include "../tools/RobinHoodMap.hpp"
#include "../core/WorldPosition.hpp"

#include <random>
#include <string>
#include <limits>
#include <cmath>

namespace cse498 {

/**
 * @brief A simple agent that tries to swarm around to a target area 
 * by talking to other agents
 * 
 * This agent maintains a map of known locations items or other points of interest and their positions.
 * It shares this knowledge with nearby agents and uses it to navigate towards a specified target.
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


public:
    SwarmingAgent(size_t id, const std::string & name, const WorldBase & world)
        : AgentBase(id, name, world), rng(static_cast<unsigned>(id)) { }

    ~SwarmingAgent() override = default;

    bool Initialize() override {
      return HasAction("up") && HasAction("down") &&
             HasAction("left") && HasAction("right");
    }

    /// Set the ID of the target location this agent is seeking.
    SwarmingAgent & SetTarget(const size_t & id) {
      target_id = id;
      return *this;
    }

    /// Seed this agent's knowledge with a known location.
    SwarmingAgent & AddKnownLocation(const size_t & name, WorldPosition pos) {
      known_locations.insert(name, pos);
      return *this;
    }

    /// Get the map of all locations this agent knows about (for knowledge sharing).
    [[nodiscard]] const RobinHoodMap<size_t, WorldPosition> & GetKnownLocations() const {
      return known_locations;
    }

    [[nodiscard]] size_t SelectAction(const WorldGrid & grid) override;
};

} // namespace cse498
