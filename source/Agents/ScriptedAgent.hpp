/**
 * @file ScriptedAgent.hpp
 * @brief Deterministic step-based agent used for simple scripted movement.
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

/**
 * @brief Agent that cycles through a fixed down/right/up/left movement pattern.
 *
 * ScriptedAgent is useful as a deterministic baseline in demos and tests. It
 * works with any data class that exposes either a `position` member or a `pos`
 * member containing a WorldPosition.
 *
 * @tparam DataClass Agent state type accepted by StepAgentBase.
 */
template <IsDataClass DataClass>
class ScriptedAgent : public StepAgentBase<DataClass> {
 protected:
  /// Current index into the four-step movement cycle.
  size_t step_index = 0;

 public:
  /**
   * @brief Construct a scripted agent with initial state and stable ID.
   * @param initial_state Initial data object stored by the base agent.
   * @param id Stable ID assigned by the world.
   */
  ScriptedAgent(DataClass initial_state, size_t id)
      : StepAgentBase<DataClass>(initial_state, id) {}

  /**
   * @brief Default destructor.
   */
  ~ScriptedAgent() = default;

  /**
   * @brief Return one movement step from the scripted cycle.
   *
   * The cycle is down, right, up, left. The world decides whether the emitted
   * movement step is valid for its grid and obstacles.
   *
   * @return StepContainer containing one MovementStep.
   */
  StepContainer GetTurn() override {
    WorldPosition pos;
    if constexpr (requires { this->mData.position; }) {
      pos = this->mData.position;
    } else {
      pos = this->mData.pos;
    }
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

  /**
   * @brief Ignore goal assignment.
   *
   * ScriptedAgent is intentionally deterministic and does not path toward a
   * caller-provided goal.
   *
   * @param pos Unused target position.
   */
  void SetGoal([[maybe_unused]] WorldPosition pos) override {}
};
// clang-format on
}  // End of namespace cse498
