#pragma once

#include <memory>
#include <vector>

#include "StepAgentBase.hpp"

namespace cse498 {

template <typename Dataclass> class WorldBase {

  using Agent = StepAgentBase<Dataclass>;
  using AgentPtr = std::shared_ptr<Agent>;

protected:
  /// Whether the simulation is running
  bool run_over = false;
  /// Collection of agents
  std::vector<AgentPtr> agent_set;

public:
  WorldBase() = default;
  virtual ~WorldBase() = default;
};

} // namespace cse498
