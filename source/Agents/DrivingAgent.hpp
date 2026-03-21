/**
 * Written by Claude.
 * A simple agent meant for the TrafficWorld demo, which is a bit like PacingAgent,
 * but with direction handled differently using StateGridPosition.
 * It walks in whatever direction it's currently pointing, then turns at corners.
 *
 * Position is ultimately managed by the world via Entity::Location (a WorldPosition),
 * but this agent keeps a StateGridPosition internally to track its facing direction.
 * The facing direction determines which "up"/"down"/"left"/"right" action is issued
 * each turn. When a forward move fails (action_result == 0), the agent turns right;
 * if a second forward attempt also fails, it turns right again, and so on, until it
 * finds a passable direction or has rotated fully.
 */

#pragma once

#include <cassert>

#include "../core/AgentBase.hpp"
#include "../tools/StateGridPosition.hpp"

// clang-format off
namespace cse498 {

  class DrivingAgent : public AgentBase {
  protected:
    StateGridPosition grid_pos;      ///< Tracks facing direction (and a local copy of position).
    size_t turn_attempts = 0;        ///< How many consecutive turns we've tried without moving.
    static constexpr size_t MAX_TURN_ATTEMPTS = 4;  ///< Full rotation = no way forward.

  public:
    DrivingAgent(size_t id, const std::string & name, const WorldBase & world)
      : AgentBase(id, name, world) { }
    ~DrivingAgent() = default;

    /// Set the agent's initial facing direction.
    DrivingAgent & SetDirection(Direction dir) {
      grid_pos.SetDirection(dir);
      return *this;
    }

    /// Get the agent's current facing direction.
    [[nodiscard]] Direction GetDirection() const {
      return grid_pos.GetDirection();
    }

    /// Set the agent's initial grid position (col, row) and optional direction.
    DrivingAgent & SetGridPosition(size_t col, size_t row,
                                   Direction dir = Direction::North) {
      grid_pos = StateGridPosition(col, row, dir);
      return *this;
    }

    /// @brief This agent needs the four directional actions to function.
    /// @return Success: are required actions available?
    bool Initialize() override {
      return HasAction("up") && HasAction("down")
          && HasAction("left") && HasAction("right");
    }

    /// @brief Choose the action to take on this turn.
    ///
    /// Strategy: try to move forward (in the current facing direction).
    /// If the last forward move failed, turn right and try again.
    /// After a full rotation with no success, just attempt forward anyway
    /// (the world will report failure and the agent stays put).
    size_t SelectAction(const WorldGrid & /* grid */) override
    {
      // If the last action failed, turn right and note the attempt.
      if (action_result == 0) {
        grid_pos.TurnRight();
        ++turn_attempts;
        // After a full rotation, reset counter so we don't spin forever.
        if (turn_attempts >= MAX_TURN_ATTEMPTS) {
          turn_attempts = 0;
        }
      } else {
        // Successful move — reset turn counter.
        turn_attempts = 0;
      }

      // Issue the action corresponding to the current facing direction.
      return GetActionForCurrentDirection();
    }

  private:
    /// @brief Map the current facing direction to the matching action ID.
    [[nodiscard]] size_t GetActionForCurrentDirection() const {
      switch (grid_pos.GetDirection()) {
        case Direction::North: return action_map.at("up");
        case Direction::South: return action_map.at("down");
        case Direction::West:  return action_map.at("left");
        case Direction::East:  return action_map.at("right");
        default:
          assert(false && "Invalid direction in DrivingAgent");
          return 0;
      }
    }
  };

// clang-format on
}  // End of namespace cse498
