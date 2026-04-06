/**
 * Initially written by Claude, but modified substantially afterward.
 * A simple agent meant for the TrafficWorld demo, which is a bit like
 * PacingAgent, but with direction handled differently using StateGridPosition.
 * It walks in whatever direction it's currently pointing, then turns at
 * corners.
 *
 * Position is ultimately managed by the world via Entity::Location (a
 * WorldPosition), but this agent keeps a StateGridPosition internally to track
 * its facing direction. The facing direction determines which
 * "up"/"down"/"left"/"right" action is issued each turn.
 */

#pragma once

#include <cassert>
#include <map>
// #include <print>

#include "../core/AgentBase.hpp"
#include "../core/WorldBase.hpp"
#include "../tools/StateGridPosition.hpp"
#include "../tools/WeightedSet.hpp"

namespace cse498 {

constexpr int GO_FORWARD = 0;
constexpr int GO_LEFT = 1;
constexpr int GO_RIGHT = 2;

class DrivingAgent : public AgentBase {
 protected:
  StateGridPosition
      grid_pos{};  ///< Tracks facing direction (and a local copy of position).
  // The right thing to do IMO would be to just make GetSymbol virtual
  // but I'll avoid interfering with core files for now.
  const std::map<Direction, char> direction_symbols = {{Direction::North, '^'},
                                                       {Direction::East, '>'},
                                                       {Direction::South, 'v'},
                                                       {Direction::West, '<'}};
  size_t road_cell_id{};
  size_t traffic_light_vertical_id{};
  size_t traffic_light_horizontal_id{};

  size_t spawn_cell_id{};
  size_t destination_cell_id{};

  // The destination that is assigned at spawn, if it reaches this it despawns
  StateGridPosition destination_pos{};
  bool reached_destination = false;

  std::string colour{};  ///< ANSI escape code for this agent's display colour.

  WeightedSet<int> move_options{};

 public:
  DrivingAgent(size_t id, const std::string &name, const WorldBase &world)
      : AgentBase(id, name, world) {
    SetDirection(Direction::East);
    road_cell_id = world.GetGrid().GetCellTypeID("road");
    spawn_cell_id = world.GetGrid().GetCellTypeID("spawn");
    destination_cell_id = world.GetGrid().GetCellTypeID("destination");
    traffic_light_vertical_id =
        world.GetGrid().GetCellTypeID("traffic_light_vertical");
    traffic_light_horizontal_id =
        world.GetGrid().GetCellTypeID("traffic_light_horizontal");
  }
  ~DrivingAgent() = default;

  /// @brief Set the agent's initial facing direction.
  DrivingAgent &SetDirection(Direction dir) {
    grid_pos.SetDirection(dir);
    SetSymbol(direction_symbols.at(grid_pos.GetDirection()));
    return *this;
  }

  [[nodiscard]] bool GetReachedDestination() const {
    return reached_destination;
  }
  void SetReachedDestination(bool reached) { reached_destination = reached; }

  /// @brief Get the agent's current facing direction.
  [[nodiscard]] Direction GetDirection() const {
    return grid_pos.GetDirection();
  }

  /// @brief Set the agent's initial grid position (col, row) and optional
  /// direction.
  DrivingAgent &SetGridPosition(size_t col, size_t row,
                                Direction dir = Direction::North) {
    grid_pos = StateGridPosition(col, row, dir);
    SetSymbol(direction_symbols.at(grid_pos.GetDirection()));
    return *this;
  }

  DrivingAgent &SetGridDestination(size_t col, size_t row) {
    destination_pos = StateGridPosition(col, row);
    return *this;
  }

  /// @brief This agent needs the four directional actions to function.
  /// @return Success: are required actions available?
  bool Initialize() override {
    return HasAction("up") && HasAction("down") && HasAction("left") &&
           HasAction("right") && HasAction("stay");
  }

  bool IsTraversableTerrain(const size_t cell_id) const {
    return cell_id == road_cell_id || cell_id == traffic_light_vertical_id ||
           cell_id == traffic_light_horizontal_id || cell_id == spawn_cell_id ||
           cell_id == destination_cell_id;
  }

  /// @brief Choose the action to take on this turn.
  ///
  /// Strategy: try to move forward (in the current facing direction).
  /// If the last forward move failed, turn left and try again.
  size_t SelectAction(const WorldGrid &grid) override {
    grid_pos.SetLocation(GetLocation().AsWorldPosition());

    // Stay put when the world signals that we were blocked last turn, e.g. by a
    // traffic light.
    if (action_result == 0) {
      return action_map["stay"];
    }

    StateGridPosition forward_pos = grid_pos.GetForwardPosition();
    StateGridPosition left_pos = grid_pos.GetLeftwardPosition();
    StateGridPosition right_pos = grid_pos.GetRightwardPosition();
    bool can_move_forward =
        IsTraversableTerrain(grid[forward_pos.AsWorldPosition()]);
    bool can_move_left = IsTraversableTerrain(grid[left_pos.AsWorldPosition()]);
    bool can_move_right =
        IsTraversableTerrain(grid[right_pos.AsWorldPosition()]);
    if (!can_move_forward && !can_move_left && !can_move_right) {
      TurnLeft();
      TurnLeft();
    } else {
      move_options.Clear();

      if (can_move_forward) move_options.Insert(GO_FORWARD, 1.0);
      if (can_move_left) move_options.Insert(GO_LEFT, 1.0);
      if (can_move_right) move_options.Insert(GO_RIGHT, 1.0);

      int move = move_options.GetRandomElement().value();
      if (move == GO_LEFT) {
        TurnLeft();
      } else if (move == GO_RIGHT) {
        TurnRight();
      }
    }

    return GetActionForCurrentDirection();
  }

  void TurnLeft() {
    grid_pos.TurnLeft();
    SetSymbol(direction_symbols.at(grid_pos.GetDirection()));
  }

  void TurnRight() {
    grid_pos.TurnRight();
    SetSymbol(direction_symbols.at(grid_pos.GetDirection()));
  }
  DrivingAgent &SetColour(const std::string &c) {
    colour = c;
    return *this;
  }
  [[nodiscard]] const std::string &GetColour() const { return colour; }

  // In DrivingAgent, public section:
  [[nodiscard]] const StateGridPosition &GetDestination() const {
    return destination_pos;
  }

 private:
  /// @brief Map the current facing direction to the matching action ID.
  [[nodiscard]] size_t GetActionForCurrentDirection() const {
    switch (grid_pos.GetDirection()) {
      case Direction::North:
        return action_map.at("up");
      case Direction::South:
        return action_map.at("down");
      case Direction::West:
        return action_map.at("left");
      case Direction::East:
        return action_map.at("right");
      default:
        assert(false && "Invalid direction in DrivingAgent");
        return 0;
    }
  }
};

}  // End of namespace cse498
