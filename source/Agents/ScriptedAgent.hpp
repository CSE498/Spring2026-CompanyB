/**
 * Demo Scripting Module
 **/

#pragma once

#include <cassert>

#include "Worlds/Group11DummyData.hpp"
#include "core/Step.hpp"
#include "core/StepAgentBase.hpp"
#include "core/WorldPosition.hpp"
#include "core/core.hpp"

namespace cse498 {

using Concepts::IsDataClass;
using cse498::steps::MovementStep;

template <IsDataClass DataClass>
class ScriptedAgent : public StepAgentBase<DataClass> {
 protected:
  size_t step_index = 0;

 public:
  ScriptedAgent(DataClass initial_state, size_t id)
      : StepAgentBase<DataClass>(initial_state, id) {}
  ~ScriptedAgent() = default;

  /// Choose the action to take a step in the appropriate direction.
  StepContainer GetTurn() override {
    WorldPosition pos = this->GetState().pos;
    StepContainer container{};

    switch (step_index % 4) {
      case 0:
        container.add_step(MovementStep{pos.Down()});
        break;
      case 1:
        container.add_step(MovementStep{pos.Right()});
        break;
      case 2:
        container.add_step(MovementStep{pos.Up()});
        break;
      case 3:
        container.add_step(MovementStep{pos.Left()});
        break;
    }

    step_index++;
    return container;
  }

  void SetGoal([[maybe_unused]] WorldPosition pos) override {}
};
// clang-format on
}  // End of namespace cse498
