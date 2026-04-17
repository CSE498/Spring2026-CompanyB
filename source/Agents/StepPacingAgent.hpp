/**
 * StepPacingAgent.hpp
 * @brief A pacing agent that uses the step-based world system.
 *
 * Walks back and forth along a row or column, emitting a single MovementStep
 * each turn. Direction reversal is detected by comparing the current position
 * against the last submitted target — if they differ the move was blocked.
 **/

#pragma once

#include <optional>

#include "../core/DiseaseData.hpp"
#include "../core/Step.hpp"
#include "../core/StepAgentBase.hpp"

namespace cse498 {

class StepPacingAgent : public StepAgentBase<DiseaseData> {
 protected:
  bool vertical = true;  ///< true = moves down/up; false = right/left
  bool reverse = false;  ///< true = moving back (up or left)

  /// Position we requested last turn; used to detect a blocked move.
  std::optional<WorldPosition> last_target{};

 public:
  explicit StepPacingAgent(DiseaseData data)
      : StepAgentBase<DiseaseData>(data) {}
  ~StepPacingAgent() override = default;

  StepPacingAgent& SetHorizontal() {
    vertical = false;
    return *this;
  }
  StepPacingAgent& SetVertical() {
    vertical = true;
    return *this;
  }

  /// SetGoal is unused for a pacing agent but required by the interface.
  void SetGoal(WorldPosition /*position*/) override {}

  /// Return a StepContainer with a single MovementStep.
  /// If the previous target was not reached (wall), reverse direction first.
  [[nodiscard]] steps::StepContainer GetTurn() override {
    using namespace cse498::steps;

    WorldPosition pos = mData.position;

    // If our last move was blocked (position unchanged), flip direction.
    if (last_target.has_value() && pos != last_target.value()) {
      reverse = !reverse;
    }

    WorldPosition target =
        vertical ? (reverse ? pos.Up() : pos.Down())
                 : (reverse ? pos.Left() : pos.Right());

    last_target = target;

    StepContainer steps;
    steps.add_step(MovementStep{target});
    return steps;
  }
};

}  // namespace cse498
