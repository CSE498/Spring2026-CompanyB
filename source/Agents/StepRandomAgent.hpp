/**
 * StepRandomAgent.hpp
 * @brief An agent that moves randomly each tick using the step-based system.
 *
 * On each turn, picks one of the four cardinal directions at random and emits
 * a single MovementStep. The world rejects the move if the target cell is a
 * wall or quarantine zone; the agent simply stays put that tick and tries a
 * fresh random direction next turn.
 **/

#pragma once

#include <random>

#include "../core/DiseaseData.hpp"
#include "../core/Step.hpp"
#include "../core/StepAgentBase.hpp"

namespace cse498 {

class StepRandomAgent : public StepAgentBase<DiseaseData> {
  mutable std::mt19937 rng;
  std::uniform_int_distribution<int> dir_dist{0, 3};

 public:
  StepRandomAgent(DiseaseData data, size_t id)
      : StepAgentBase<DiseaseData>(data, id),
        // Seed from starting position for reproducibility while keeping
        // different agents out of sync with each other.
        rng(static_cast<unsigned>(data.position.CellX() * 2654435761u ^
                                  data.position.CellY() * 2246822519u)) {}

  ~StepRandomAgent() override = default;

  void SetGoal(WorldPosition /*position*/) override {}

  [[nodiscard]] steps::StepContainer GetTurn() override {
    using namespace cse498::steps;

    WorldPosition pos = mData.position;
    WorldPosition target;
    switch (dir_dist(rng)) {
      case 0: target = pos.Up();    break;
      case 1: target = pos.Down();  break;
      case 2: target = pos.Left();  break;
      default: target = pos.Right(); break;
    }

    StepContainer steps;
    steps.add_step(MovementStep{target});
    return steps;
  }
};

}  // namespace cse498
