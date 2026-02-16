/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief StateGridPosition movement and move validty methods implementation.
 **/

#include "StateGridPosition.hpp"
#include "StateGrid.hpp"  // Need to wait on this

namespace cse498 {

  bool StateGridPosition::MoveForward(const StateGrid& grid) {
    StateGridPosition new_pos = GetForwardPosition();
    // StateGrid uses int, so static cast to int
    // StateGrid takes row then column
    // Check if position is in bounds and is traversable
    if (grid.inBounds(static_cast<int>(new_pos.CellY()), 
                      static_cast<int>(new_pos.CellX()))) {
      x = new_pos.x;
      y = new_pos.y;
      return true;
    }
    return false;
  }

  bool StateGridPosition::MoveBackward(const StateGrid& grid) {
    StateGridPosition new_pos = GetBackwardPosition();

    if (grid.inBounds(static_cast<int>(new_pos.CellY()), 
                      static_cast<int>(new_pos.CellX()))) {
      x = new_pos.x;
      y = new_pos.y;
      return true;
    }
    return false;
  }

} // End of namespace cse498
