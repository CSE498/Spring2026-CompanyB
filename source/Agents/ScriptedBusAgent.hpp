/**
 * Written entirely by claude
 * needs to be vetted throughly
 * */

#pragma once

#include "../core/AgentData.hpp"
#include "../core/Step.hpp"
#include "../Agents/ScriptedAgent.hpp"
#include "../core/StepAgentBase.hpp"

namespace cse498 {

/**
 * @brief Scripted bus agent that circles the road network.
 *
 * Each turn the bus prefers to turn right relative to its current heading.
 * If right is blocked it tries straight, then left, then reverse as a last
 * resort.  On any closed road loop this produces consistent clockwise
 * circulation without any goal or destination.
 */
class ScriptedBusAgent : public StepAgentBase<TrafficData> {
 public:
  ScriptedBusAgent(TrafficData data, size_t id)
      : StepAgentBase<TrafficData>(data, id) {}

  void SetGoal([[maybe_unused]] WorldPosition pos) override {}

  [[nodiscard]] steps::StepContainer GetTurn() override {
    steps::StepContainer container{};

    if (!this->mData.is_active) return container;

    WorldPosition pos = this->mData.position;
    Direction dir = this->mData.direction;

    WorldPosition right_pos = TurnRight(pos, dir);
    WorldPosition straight_pos = GoStraight(pos, dir);
    WorldPosition left_pos = TurnLeft(pos, dir);
    WorldPosition back_pos = GoBackward(pos, dir);

    using steps::ConditionalStep;
    using steps::InfoHandler;
    using steps::InfoStep;
    using steps::MovementStep;
    using steps::StepContainer;
    using steps::StepErr;

    auto make_cond = []() {
      return ConditionalStep{InfoHandler(
          [](bool b) -> std::expected<bool, StepErr> { return b; })};
    };

    // Innermost fallback: left, else reverse
    StepContainer left_or_back;
    left_or_back.add_step(
        InfoStep{InfoStep::Aspect::LOC_AVAIL, left_pos}, make_cond(),
        MovementStep{left_pos}, MovementStep{back_pos});

    // Middle: straight, else (left / reverse)
    StepContainer straight_or_left;
    straight_or_left.add_step(
        InfoStep{InfoStep::Aspect::LOC_AVAIL, straight_pos}, make_cond(),
        MovementStep{straight_pos}, std::move(left_or_back));

    // Outer: right, else (straight / left / reverse)
    container.add_step(InfoStep{InfoStep::Aspect::LOC_AVAIL, right_pos},
                       make_cond(), MovementStep{right_pos},
                       std::move(straight_or_left));

    return container;
  }

 private:
  static WorldPosition TurnRight(WorldPosition pos, Direction dir) {
    switch (dir) {
      case Direction::North: return pos.Right();
      case Direction::East:  return pos.Down();
      case Direction::South: return pos.Left();
      case Direction::West:  return pos.Up();
    }
    return pos;
  }

  static WorldPosition GoStraight(WorldPosition pos, Direction dir) {
    switch (dir) {
      case Direction::North: return pos.Up();
      case Direction::East:  return pos.Right();
      case Direction::South: return pos.Down();
      case Direction::West:  return pos.Left();
    }
    return pos;
  }

  static WorldPosition TurnLeft(WorldPosition pos, Direction dir) {
    switch (dir) {
      case Direction::North: return pos.Left();
      case Direction::East:  return pos.Up();
      case Direction::South: return pos.Right();
      case Direction::West:  return pos.Down();
    }
    return pos;
  }

  static WorldPosition GoBackward(WorldPosition pos, Direction dir) {
    switch (dir) {
      case Direction::North: return pos.Down();
      case Direction::East:  return pos.Left();
      case Direction::South: return pos.Up();
      case Direction::West:  return pos.Right();
    }
    return pos;
  }
};

}  // namespace cse498
