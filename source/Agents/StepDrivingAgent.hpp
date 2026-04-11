/**
 * Written by Claude (based on a human-written rough sketch, and with
 * inspiration taken from SwarmingAgent.)
 *
 * Port of DrivingAgent onto the StepAgentBase / StepContainer framework
 * documented in steps.org and implemented in core/Step.hpp.
 *
 * On each turn the agent inspects the three cells adjacent to its current
 * facing direction (forward / left / right), shuffles them into a random
 * preference order, and builds a nested conditional chain that asks the
 * world whether each candidate is available in turn. The agent moves to
 * the first available candidate and rotates its facing direction to
 * match; if none of the three are available, the container falls through
 * without emitting a MovementStep and the agent stays put for this turn.
 *
 * Unlike the original DrivingAgent, this version leaves all "what counts
 * as traversable terrain" logic on the world side via
 * InfoStep::Aspect::LOC_AVAIL, rather than checking cell type IDs
 * directly on the agent.
 */

#pragma once

#include <algorithm>
#include <array>
#include <expected>
#include <random>
#include <utility>

#include "../Worlds/TrafficData.hpp"
#include "../core/Step.hpp"
#include "../core/StepAgentBase.hpp"
#include "../core/WorldPosition.hpp"
#include "../tools/StateGridPosition.hpp"

namespace cse498 {

class StepDrivingAgent : public StepAgentBase<TrafficData> {
 private:
  std::mt19937 rng{std::random_device{}()};

  /// A single directional move the agent might take this turn: where it
  /// would end up, and which way it would be facing after the move.
  struct Candidate {
    WorldPosition target;
    Direction new_direction;
  };

  [[nodiscard]] static Direction RotatedLeft(Direction d) {
    return static_cast<Direction>((static_cast<int>(d) + 3) % 4);
  }

  [[nodiscard]] static Direction RotatedRight(Direction d) {
    return static_cast<Direction>((static_cast<int>(d) + 1) % 4);
  }

  /// Step one cell in the given direction from `pos`. Uses WorldPosition's
  /// signed-double arithmetic rather than StateGridPosition so that we
  /// don't have to worry about size_t underflow at the grid edges -- the
  /// world is responsible for reporting LOC_AVAIL = false on any
  /// out-of-bounds target we propose.
  [[nodiscard]] static WorldPosition StepFrom(WorldPosition pos, Direction d) {
    switch (d) {
      case Direction::North:
        return pos.Up();
      case Direction::East:
        return pos.Right();
      case Direction::South:
        return pos.Down();
      case Direction::West:
        return pos.Left();
    }
    return pos;
  }

  /// Build the three directional candidates (forward / left / right)
  /// from the agent's current position and facing.
  [[nodiscard]] std::array<Candidate, 3> BuildCandidates() const {
    const WorldPosition& pos = mData.position;
    const Direction dir = mData.direction;

    const Direction left_dir = RotatedLeft(dir);
    const Direction right_dir = RotatedRight(dir);

    return {Candidate{StepFrom(pos, dir), dir},
            Candidate{StepFrom(pos, left_dir), left_dir},
            Candidate{StepFrom(pos, right_dir), right_dir}};
  }

  /// Build a ConditionalStep whose handler both reports whether the
  /// move should be taken (mirroring the world's LOC_AVAIL answer) and,
  /// on success, updates the agent's facing direction to match the move
  /// that is about to run. The direction update happens synchronously
  /// during step evaluation -- before the paired MovementStep executes
  /// -- so the agent's recorded facing stays consistent with the move
  /// the world is committing to.
  [[nodiscard]] steps::ConditionalStep MakeTurningConditional(
      Direction new_direction) {
    return steps::ConditionalStep{steps::InfoHandler(
        [this, new_direction](
            bool is_available) -> std::expected<bool, steps::StepErr> {
          if (is_available) {
            TrafficData updated = this->GetState();
            updated.direction = new_direction;
            this->SetState(updated);
          }
          return is_available;
        })};
  }

  /// Build a conditional that tries the given candidate; if the candidate
  /// is unavailable, execution falls through to `fallback` (which should
  /// represent the remaining, lower-priority candidates).
  [[nodiscard]] StepContainer BuildPreferenceChain(const Candidate& primary,
                                                   StepContainer&& fallback) {
    StepContainer chain{};
    chain.add_step(
        steps::InfoStep{steps::InfoStep::Aspect::LOC_AVAIL, primary.target},
        MakeTurningConditional(primary.new_direction),
        steps::MovementStep{primary.target}, std::move(fallback));
    return chain;
  }

 public:
  using StepAgentBase<TrafficData>::StepAgentBase;

  [[nodiscard]] StepContainer GetTurn() override {
    StepContainer container{};

    // Inactive agents take no steps at all, same convention as
    // SwarmingAgent.
    if (!mData.is_active) return container;

    // Compute and randomly rank the three directional candidates, so
    // that whichever open move is seen first wins (replacing the
    // WeightedSet-based random pick in the original DrivingAgent).
    auto candidates = BuildCandidates();
    std::shuffle(candidates.begin(), candidates.end(), rng);

    // Build the preference chain from the back forward:
    // third-only -> second-then-third -> first-then-rest.
    StepContainer third_only{};
    third_only.add_step(steps::InfoStep{steps::InfoStep::Aspect::LOC_AVAIL,
                                        candidates[2].target},
                        MakeTurningConditional(candidates[2].new_direction),
                        steps::MovementStep{candidates[2].target});

    StepContainer second_then_third =
        BuildPreferenceChain(candidates[1], std::move(third_only));

    container.add_step(steps::InfoStep{steps::InfoStep::Aspect::LOC_AVAIL,
                                       candidates[0].target},
                       MakeTurningConditional(candidates[0].new_direction),
                       steps::MovementStep{candidates[0].target},
                       std::move(second_then_third));

    return container;
  }

  void SetGoal(WorldPosition position) override {
    TrafficData updated = GetState();
    updated.destination = position;
    SetState(updated);
  }
};

}  // namespace cse498
