/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A specification of a coordinate position in a World.
 **/

#pragma once

#include <cassert>
#include <cmath>
#include <compare>
#include <concepts>  // For std::integral
#include <cstddef>   // For size_t

#include "./StateGrid/StateGrid.hpp"
#include "../core/WorldPosition.hpp"

namespace cse498 {
/// @brief Constitutes the direction of an agent
enum class Direction { North = 0, East = 1, South = 2, West = 3 };

/// @class StateGridPosition
/// @brief Monitors position and orientation of an agent in a StateGrid
/// Allows for movement of agent and basic distance calculations.
class StateGridPosition {
 private:
  size_t col = 0;
  size_t row = 0;
  Direction direction_facing = Direction::North;

 public:
  // -- Constructors --
  StateGridPosition() = default;
  StateGridPosition(
      size_t col, size_t row,
      Direction direction_facing =
          Direction::North)  // already defaults to North, but shown here for
                             // better code readability
      : col(col), row(row), direction_facing(direction_facing) {}

  /// @brief Construct from integral trowpes.
  /// Allows for using constructor with other trowpes like ints.
  template <std::integral T, std::integral U>
  StateGridPosition(T col, U row, Direction direction_facing = Direction::North)
      : StateGridPosition(static_cast<size_t>(col), static_cast<size_t>(row),
                          direction_facing) {}

  StateGridPosition(const StateGridPosition&) = default;
  StateGridPosition& operator=(const StateGridPosition&) = default;

  WorldPosition AsWorldPosition() const {
    return WorldPosition(col, row);
  }

  // -- Movement Functions --

  void SetLocation(WorldPosition pos) {
    col = pos.CellX();
    row = pos.CellY();
  }

  bool MoveForward(const StateGrid& grid) {
    if (!IsValidForwardMove(grid)) return false;
    StateGridPosition new_pos = GetForwardPosition();
    col = new_pos.col;
    row = new_pos.row;
    return true;
  }

  bool MoveBackward(const StateGrid& grid) {
    if (!IsValidBackwardMove(grid)) return false;
    StateGridPosition new_pos = GetBackwardPosition();
    col = new_pos.col;
    row = new_pos.row;
    return true;
  }

  /// @brief Turn agent 90 degrees to the left
  void TurnLeft() {
    int dir_value = static_cast<int>(direction_facing);
    direction_facing = static_cast<Direction>((dir_value + 3) % 4);
  }

  /// @brief Turn agent 90 degrees to the right
  void TurnRight() {
    int dir_value = static_cast<int>(direction_facing);
    direction_facing = static_cast<Direction>((dir_value + 1) % 4);
  }

  /// @brief Check if a forward move is allowed without moving
  /// @param grid Reference to the StateGrid
  /// @return true if forward movement is allowed, false otherwise
  [[nodiscard]] bool IsValidForwardMove(const StateGrid& grid) const {
    // Check edge boundaries before calling GetForwardPosition to avoid size_t
    // underflow
    if (direction_facing == Direction::North && row == 0) return false;
    if (direction_facing == Direction::West && col == 0) return false;
    StateGridPosition new_pos = GetForwardPosition();
    return grid.inBounds(static_cast<int>(new_pos.row),
                         static_cast<int>(new_pos.col));
  }

  /// @brief Check if a backward move is allowed without moving
  /// @param grid Reference to the StateGrid
  /// @return true if backward movement is allowed, false otherwise
  [[nodiscard]] bool IsValidBackwardMove(const StateGrid& grid) const {
    // Check edge boundaries before calling GetBackwardPosition to avoid size_t
    // underflow
    if (direction_facing == Direction::East && col == 0) return false;
    if (direction_facing == Direction::South && row == 0) return false;
    StateGridPosition new_pos = GetBackwardPosition();
    return grid.inBounds(static_cast<int>(new_pos.row),
                         static_cast<int>(new_pos.col));
  }

  // -- Distance Functions --

  /// @brief Calculate Euclidean distance to another position
  /// @param other The other position to measure distance to
  /// @return The Euclidean distance as a double
  [[nodiscard]] double EuclideanDistance(const StateGridPosition& other) const {
    double dcol = static_cast<double>(col) - static_cast<double>(other.col);
    double drow = static_cast<double>(row) - static_cast<double>(other.row);
    return std::sqrt(dcol * dcol + drow * drow);
  }

  // -- Accessors --

  /// @brief Get col coordinate
  [[nodiscard]] double Col() const { return col; }

  /// @brief Get row coordinate
  [[nodiscard]] double Row() const { return row; }

  /// @brief Get current facing direction
  [[nodiscard]] Direction GetDirection() const { return direction_facing; }

  // -- Mutators --

  /// @brief Set the facing direction
  /// @param dir New direction to face
  /// @throws std::invalid_argument if direction is invalid
  void SetDirection(Direction dir) { direction_facing = dir; }

  // -- Position Queries --

  /// @brief Get position one step forward
  [[nodiscard]] StateGridPosition GetForwardPosition() const {
    switch (direction_facing) {
      case Direction::North:
        assert(row > 0 &&
               "Cannot move North from row 0");  // underflows to SIZE_MAX if
                                                 // row == 0
        return StateGridPosition(col, row - 1, direction_facing);
      case Direction::East:
        return StateGridPosition(col + 1, row, direction_facing);
      case Direction::South:
        return StateGridPosition(col, row + 1, direction_facing);
      case Direction::West:
        assert(col > 0 &&
               "Cannot move West from col 0");  // underflows to SIZE_MAX if col
                                                // == 0
        return StateGridPosition(col - 1, row, direction_facing);
      default:
        assert(false && "Invalid direction in GetForwardPosition");
        return *this;
    }
  }

  /// @brief Get position one step backward
  [[nodiscard]] StateGridPosition GetBackwardPosition() const {
    switch (direction_facing) {
      case Direction::North:
        return StateGridPosition(col, row + 1, direction_facing);
      case Direction::East:
        assert(col > 0 &&
               "Cannot move East backward from col 0");  // underflows to
                                                         // SIZE_MAX if col == 0
        return StateGridPosition(col - 1, row, direction_facing);
      case Direction::South:
        assert(
            row > 0 &&
            "Cannot move South backward from row 0");  // underflows to SIZE_MAX
                                                       // if row == 0
        return StateGridPosition(col, row - 1, direction_facing);
      case Direction::West:
        return StateGridPosition(col + 1, row, direction_facing);
      default:
        assert(false && "Invalid direction in GetBackwardPosition");
        return *this;
    }
  }

  [[nodiscard]] StateGridPosition GetLeftwardPosition() const {
    switch (direction_facing) {
      case Direction::North:
        return StateGridPosition(col - 1, row, direction_facing);
      case Direction::East:
        return StateGridPosition(col, row - 1, direction_facing);
      case Direction::South:
        return StateGridPosition(col + 1, row, direction_facing);
      case Direction::West:
        return StateGridPosition(col, row + 1, direction_facing);
    }
  }

  [[nodiscard]] StateGridPosition GetRightwardPosition() const {
    switch (direction_facing) {
      case Direction::North:
        return StateGridPosition(col + 1, row, direction_facing);
      case Direction::East:
        return StateGridPosition(col, row + 1, direction_facing);
      case Direction::South:
        return StateGridPosition(col - 1, row, direction_facing);
      case Direction::West:
        return StateGridPosition(col, row - 1, direction_facing);
    }
  }

  // @brief Get position at requested offset
  [[nodiscard]] StateGridPosition GetOffset(long long offset_col,
                                            long long offset_row) const {
    // Asserts are for size_t underflow
    assert(static_cast<long long>(col) + offset_col >= 0 &&
           "offset_col would underflow col");
    assert(static_cast<long long>(row) + offset_row >= 0 &&
           "offset_row would underflow row");
    return StateGridPosition(
        static_cast<size_t>(static_cast<long long>(col) + offset_col),
        static_cast<size_t>(static_cast<long long>(row) + offset_row),
        direction_facing);
  }

  // -- Comparison --

  // @brief Compare positions (ignores direction)
  [[nodiscard]] bool SameCell(const StateGridPosition& other) const {
    return col == other.col && row == other.row;
  }

  // @brief Returns true if positions are in the same cell and facing the same
  // direction
  bool operator==(const StateGridPosition& other) const {
    return SameCell(other) && direction_facing == other.direction_facing;
  }

  // @brief Returns false if positions are in the same cell and facing the same
  // direction
  bool operator!=(const StateGridPosition& other) const {
    return !(*this == other);
  }
};
}  // End of namespace cse498
