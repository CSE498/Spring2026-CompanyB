/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A specification of a coordinate position in a World.
 **/

#pragma once

#include <cassert>
#include <cmath>
#include <concepts>   // For std::integral
#include <cstddef>    // For size_t
#include <stdexcept>
#include <compare>    // For operator<=>

namespace cse498 {

  /// @class MockStateGrid
  /// @brief Bare minimum to allow for compilation
  class MockStateGrid {
  public:
    MockStateGrid(int width, int height) {}

    bool inBounds(int row, int col) const {
      return true; 
    }
  };

  using StateGrid = MockStateGrid;

}

namespace cse498 {

  // Forward declaration
  //class StateGrid;

  /// @brief Constitutes the direction of an agent
  enum class Direction {
    North = 0,
    East = 1,
    South = 2,
    West = 3
  };

  /// @class StateGridPosition
  /// @brief Monitors position and orientation of an agent in a StateGrid
  /// Allows for movement of agent and basic distance calculations.
  class StateGridPosition {
  private:
    double x = 0.0;
    double y = 0.0;
    Direction direction_facing = Direction::North;

    /// @brief Validates that a direction enum value is valid
    static void ValidateDirection(Direction dir) {
      int dir_value = static_cast<int>(dir);
      assert(dir_value >= 0 && dir_value <= 3 && "Invalid direction enum value");
    }

    /// @brief Validates that coordinates are non-negative
    static void ValidateCoordinates(double x, double y) {
      assert(x >= 0.0 && y >= 0.0 && "Coordinate values cannot be negative");
    }

  public:
    // -- Constructors --
    StateGridPosition() = default;
    StateGridPosition(double x, double y, Direction direction_facing = Direction::North)
      : x(x), y(y), direction_facing(direction_facing) {
      ValidateCoordinates(x, y);
      ValidateDirection(direction_facing);
    }

    /// @brief Construct from integral types.
    /// Allows for using constructor with other types like ints.
    template <std::integral T, std::integral U>
    StateGridPosition(T x, U y, Direction direction_facing = Direction::North)
      : StateGridPosition(static_cast<double>(x), static_cast<double>(y), direction_facing) {}

    StateGridPosition(const StateGridPosition&) = default;
    StateGridPosition& operator=(const StateGridPosition&) = default;

    // -- Movement Functions --
    bool MoveForward(const StateGrid& grid);
    bool MoveBackward(const StateGrid& grid);

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

    /// @brief Turn agent 180 degrees around
    void TurnAround() {
      int dir_value = static_cast<int>(direction_facing);
      direction_facing = static_cast<Direction>((dir_value + 2) % 4);
    }

    /// @brief Check if a forward move is allowed without moving
    /// @param grid Reference to the StateGrid
    /// @return true if forward movement is allowed, false otherwise
    [[nodiscard]] bool IsValidForwardMove(const StateGrid& grid) const {
      StateGridPosition new_pos = GetForwardPosition();
      return grid.inBounds(static_cast<int>(new_pos.CellY()), 
                           static_cast<int>(new_pos.CellX()));
    }

    /// @brief Check if a backward move is allowed without moving
    /// @param grid Reference to the StateGrid
    /// @return true if backward movement is allowed, false otherwise
    [[nodiscard]] bool IsValidBackwardMove(const StateGrid& grid) const {
      StateGridPosition new_pos = GetBackwardPosition();
      return grid.inBounds(static_cast<int>(new_pos.CellY()), 
                           static_cast<int>(new_pos.CellX()));
    }

    // -- Distance Functions --

    /// @brief Calculate Euclidean distance to another position
    /// @param other The other position to measure distance to
    /// @return The Euclidean distance as a double
    [[nodiscard]] double EuclideanDistance(const StateGridPosition& other) const {
      double dx = x - other.x;
      double dy = y - other.y;
      return std::sqrt(dx * dx + dy * dy);
    }

    // -- Accessors --

    /// @brief Get X coordinate
    [[nodiscard]] double X() const { return x; }

    /// @brief Get Y coordinate
    [[nodiscard]] double Y() const { return y; }

    /// @brief Get X coordinate as a grid cell index
    [[nodiscard]] size_t CellX() const {
      assert(x >= 0.0);
      return static_cast<size_t>(x);
    }

    /// @brief Get Y coordinate as a grid cell index
    [[nodiscard]] size_t CellY() const {
      assert(y >= 0.0);
      return static_cast<size_t>(y);
    }

    /// @brief Get current facing direction
    [[nodiscard]] Direction GetDirection() const { return direction_facing; }

    // -- Mutators --

    /// @brief Set the facing direction
    /// @param dir New direction to face
    /// @throws std::invalid_argument if direction is invalid
    void SetDirection(Direction dir) {
      ValidateDirection(dir);
      direction_facing = dir;
    }

    // -- Position Queries --

    /// @brief Get position one step forward
    [[nodiscard]] StateGridPosition GetForwardPosition() const {
      switch (direction_facing) {
        case Direction::North: return StateGridPosition(x, y - 1.0, direction_facing);
        case Direction::East:  return StateGridPosition(x + 1.0, y, direction_facing);
        case Direction::South: return StateGridPosition(x, y + 1.0, direction_facing);
        case Direction::West:  return StateGridPosition(x - 1.0, y, direction_facing);
        default:
          assert(false && "Invalid direction in GetForwardPosition");
          return *this;
      }
    }

    /// @brief Get position one step backward
    [[nodiscard]] StateGridPosition GetBackwardPosition() const {
      switch (direction_facing) {
        case Direction::North: return StateGridPosition(x, y + 1.0, direction_facing);
        case Direction::East:  return StateGridPosition(x - 1.0, y, direction_facing);
        case Direction::South: return StateGridPosition(x, y - 1.0, direction_facing);
        case Direction::West:  return StateGridPosition(x + 1.0, y, direction_facing);
        default:
          assert(false && "Invalid direction in GetBackwardPosition");
          return *this;
      }
    }

    /// @brief Get position at requested offset
    [[nodiscard]] StateGridPosition GetOffset(double offset_x, double offset_y) const {
      return StateGridPosition(x + offset_x, y + offset_y, direction_facing);
    }

    // -- Comparison --

    /// @brief Compare positions (ignores direction)
    [[nodiscard]] bool SameCell(const StateGridPosition& other) const {
      return CellX() == other.CellX() && CellY() == other.CellY();
    }

    /// @brief Enable all comparison operators (==, !=, <, <=, >, >=)
    auto operator<=>(const StateGridPosition&) const = default;
  };

} // End of namespace cse498
