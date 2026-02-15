/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief StateGridPosition movement and move validty methods implementation.
 **/

// Doesn't compile yet
#include "StateGridPosition.hpp"
#include "StateGrid.hpp"  // Need to wait on this

namespace cse498 {

  bool StateGridPosition::MoveForward(const StateGrid& grid) {
    StateGridPosition new_pos = GetForwardPosition();

    // Determine if position exists in grid and if it is blocked by something
    if (grid.IsValidPosition(new_pos.CellX(), new_pos.CellY()) &&
        !grid.IsBlocked(new_pos.CellX(), new_pos.CellY())) {
      x = new_pos.x;
      y = new_pos.y;
      return true;
    }
    return false;
  }

  bool StateGridPosition::MoveBackward(const StateGrid& grid) {
    StateGridPosition new_pos = GetBackwardPosition();

    // Determine if position exists in grid and if it is blocked by something
    if (grid.IsValidPosition(new_pos.CellX(), new_pos.CellY()) &&
        !grid.IsBlocked(new_pos.CellX(), new_pos.CellY())) {
      x = new_pos.x;
      y = new_pos.y;
      return true;
    }
    return false;
  }

  [[nodiscard]] bool IsValidForwardMove(const StateGrid& grid) const {
    StateGridPosition new_pos = GetForwardPosition();
    return grid.IsValidPosition(new_pos.CellX(), new_pos.CellY()) &&
           !grid.IsBlocked(new_pos.CellX(), new_pos.CellY());
  }

  [[nodiscard]] bool IsValidBackwardMove(const StateGrid& grid) const {
    StateGridPosition new_pos = GetBackwardPosition();
    return grid.IsValidPosition(new_pos.CellX(), new_pos.CellY()) &&
           !grid.IsBlocked(new_pos.CellX(), new_pos.CellY());
  }

} // End of namespace cse498
