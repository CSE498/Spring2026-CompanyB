#pragma once

#include "core/AgentData.hpp"
#include "Agents/SwarmingAgent.hpp"
#include "StepTrafficWorld.hpp"

namespace cse498 {

/// Subclass adapter that exposes StepTrafficWorld's protected members so
/// the web canvas drawer can read grid and per agent state each frame,
/// and DataLog can log them
class WebTrafficWorld : public StepTrafficWorld<SwarmingAgent<cse498::TrafficData>> {
 public:
  using Base = StepTrafficWorld<SwarmingAgent<TrafficData>>;
  using Base::Base;  // inherit constructors

  [[nodiscard]] const WorldGrid& GetGrid() const { return main_grid; }
  [[nodiscard]] size_t GetNumAgents() const { return agent_set.size(); }
  [[nodiscard]] TrafficData GetAgentState(size_t id) const {
    return agent_set[id]->GetState();
  }

  [[nodiscard]] size_t GetNumSpawnedAgents() const { return num_spawned_agents; }
};

}