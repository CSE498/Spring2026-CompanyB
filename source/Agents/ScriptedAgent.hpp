/**
 * Demo Scripting Module
 **/

#pragma once

#include <cassert>

#include "../core/WorldBase.hpp"
#include "../core/AgentBase.hpp"

// clang-format off
namespace cse498 {

  using cse498::DummyAgentData;
  using cse498::steps::MovementStep;

  class ScriptedAgent : public AgentBase<DummyAgentData> {
  protected:
      int step_index = 0;
  public:
    ScriptedAgent(DummyAgentData initial_state)
      : AgentBase(initial_state) { }
    ~ScriptedAgent() = default;

    /// Choose the action to take a step in the appropriate direction.
    StepContainer GetTurn() override
    {
      WorldPosition pos = GetState().pos;
      StepContainer container;

        switch (step_index % 4) {
            case 0:
        
                container.add_step<MovementStep>(pos.Down()); // Down
                break;
            case 1:
                container.add_step<MovementStep>(pos.Right()); // Right
                break;
            case 2:
                container.add_step<MovementStep>(pos.Up()); // Up
                break;
            case 3:
                container.add_step<MovementStep>(pos.Left()); // Left
                break;
        }

        step_index++;
        return container;
    }

  };
// clang-format on
}  // End of namespace cse498
