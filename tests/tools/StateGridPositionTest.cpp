#include "catch2/catch.hpp"
#include "../../source/tools/StateGridPosition.hpp"
// #include "../../../source/core/StateGrid.hpp"  // Not ready yet

TEST_CASE("Test StateGridPosition Constructors", "[core]")
{
  cse498::StateGridPosition pos1;  // Default constructor
  cse498::StateGridPosition pos2(5.0, 10.0);  // Double constructor
  cse498::StateGridPosition pos3(3, 7);  // Template constructor
  cse498::StateGridPosition pos4(5.0, 10.0, cse498::Direction::East);  // With direction
  cse498::StateGridPosition pos5(pos2);  // Copy constructor

  // Test default values
  CHECK(pos1.X() == 0.0);
  CHECK(pos1.Y() == 0.0);
  CHECK(pos1.GetDirection() == cse498::Direction::North);

  // Test double constructor
  CHECK(pos2.X() == 5.0);
  CHECK(pos2.Y() == 10.0);
  CHECK(pos2.GetDirection() == cse498::Direction::North);

  // Test Template constructor
  CHECK(pos3.X() == 3.0);
  CHECK(pos3.Y() == 7.0);

  // Test constructor with direction
  CHECK(pos4.GetDirection() == cse498::Direction::East);

  // Test copy constructor
  CHECK(pos5.X() == pos2.X());
  CHECK(pos5.Y() == pos2.Y());
  CHECK(pos5.GetDirection() == pos2.GetDirection());
}

TEST_CASE("Test StateGridPosition Assignment", "[core]")
{
  cse498::StateGridPosition pos1(3.0, 4.0, cse498::Direction::South);
  cse498::StateGridPosition pos2;

  pos2 = pos1;  // Assignment operator

  CHECK(pos2.X() == 3.0);
  CHECK(pos2.Y() == 4.0);
  CHECK(pos2.GetDirection() == cse498::Direction::South);
}

TEST_CASE("Test StateGridPosition Accessors", "[core]")
{
  cse498::StateGridPosition pos(7.5, 9.3, cse498::Direction::West);

  // Test coordinate accessors
  CHECK(pos.X() == 7.5);
  CHECK(pos.Y() == 9.3);

  // Test cell accessors
  CHECK(pos.CellX() == 7);
  CHECK(pos.CellY() == 9);

  // Test direction accessor
  CHECK(pos.GetDirection() == cse498::Direction::West);

  // Test with large values
  cse498::StateGridPosition pos2(1000.0, 2000.0);
  CHECK(pos2.CellX() == 1000);
  CHECK(pos2.CellY() == 2000);
}

TEST_CASE("Test StateGridPosition Comparisons", "[core]")
{
  cse498::StateGridPosition pos1(5.0, 10.0, cse498::Direction::North);
  cse498::StateGridPosition pos2(5.0, 10.0, cse498::Direction::North);
  cse498::StateGridPosition pos3(5.0, 10.0, cse498::Direction::East);
  cse498::StateGridPosition pos4(3.0, 4.0, cse498::Direction::North);

  // Test equality, considers direction
  CHECK(pos1 == pos2);
  CHECK(pos1 != pos3);  // Different direction
  CHECK(pos1 != pos4);  // Different position

  // Test SameCell, ignores direction
  CHECK(pos1.SameCell(pos2));
  CHECK(pos1.SameCell(pos3));  // Same cell but different direction
  CHECK_FALSE(pos1.SameCell(pos4));  // Different cell
}

TEST_CASE("Test StateGridPosition SetDirection", "[core]")
{
  cse498::StateGridPosition pos(5.0, 5.0);

  CHECK(pos.GetDirection() == cse498::Direction::North);

  pos.SetDirection(cse498::Direction::East);
  CHECK(pos.GetDirection() == cse498::Direction::East);

  pos.SetDirection(cse498::Direction::South);
  CHECK(pos.GetDirection() == cse498::Direction::South);

  pos.SetDirection(cse498::Direction::West);
  CHECK(pos.GetDirection() == cse498::Direction::West);

  pos.SetDirection(cse498::Direction::North);
  CHECK(pos.GetDirection() == cse498::Direction::North);
}

TEST_CASE("Test StateGridPosition Turn Methods", "[core]")
{
  cse498::StateGridPosition pos(5.0, 5.0, cse498::Direction::North);

  // Test TurnRight
  pos.TurnRight();
  CHECK(pos.GetDirection() == cse498::Direction::East);
  pos.TurnRight();
  CHECK(pos.GetDirection() == cse498::Direction::South);
  pos.TurnRight();
  CHECK(pos.GetDirection() == cse498::Direction::West);
  pos.TurnRight();
  CHECK(pos.GetDirection() == cse498::Direction::North);

  // Test TurnLeft
  pos.TurnLeft();
  CHECK(pos.GetDirection() == cse498::Direction::West);
  pos.TurnLeft();
  CHECK(pos.GetDirection() == cse498::Direction::South);
  pos.TurnLeft();
  CHECK(pos.GetDirection() == cse498::Direction::East);
  pos.TurnLeft();
  CHECK(pos.GetDirection() == cse498::Direction::North);

  // Test TurnAround
  pos.TurnAround();
  CHECK(pos.GetDirection() == cse498::Direction::South);
  pos.TurnAround();
  CHECK(pos.GetDirection() == cse498::Direction::North);
}

TEST_CASE("Test StateGridPosition GetForwardPosition", "[core]")
{
  // Test North
  cse498::StateGridPosition pos_north(5.0, 5.0, cse498::Direction::North);
  auto forward_north = pos_north.GetForwardPosition();
  CHECK(forward_north.X() == 5.0);
  CHECK(forward_north.Y() == 4.0);
  CHECK(forward_north.GetDirection() == cse498::Direction::North);

  // Test East
  cse498::StateGridPosition pos_east(5.0, 5.0, cse498::Direction::East);
  auto forward_east = pos_east.GetForwardPosition();
  CHECK(forward_east.X() == 6.0);
  CHECK(forward_east.Y() == 5.0);

  // Test South
  cse498::StateGridPosition pos_south(5.0, 5.0, cse498::Direction::South);
  auto forward_south = pos_south.GetForwardPosition();
  CHECK(forward_south.X() == 5.0);
  CHECK(forward_south.Y() == 6.0);

  // Test West
  cse498::StateGridPosition pos_west(5.0, 5.0, cse498::Direction::West);
  auto forward_west = pos_west.GetForwardPosition();
  CHECK(forward_west.X() == 4.0);
  CHECK(forward_west.Y() == 5.0);

  // Verify original position unchanged
  CHECK(pos_north.X() == 5.0);
  CHECK(pos_north.Y() == 5.0);
}

TEST_CASE("Test StateGridPosition GetBackwardPosition", "[core]")
{
  // Test North (backward is South direction)
  cse498::StateGridPosition pos_north(5.0, 5.0, cse498::Direction::North);
  auto backward_north = pos_north.GetBackwardPosition();
  CHECK(backward_north.X() == 5.0);
  CHECK(backward_north.Y() == 6.0);

  // Test East (backward is West direction)
  cse498::StateGridPosition pos_east(5.0, 5.0, cse498::Direction::East);
  auto backward_east = pos_east.GetBackwardPosition();
  CHECK(backward_east.X() == 4.0);
  CHECK(backward_east.Y() == 5.0);

  // Test South (backward is North direction)
  cse498::StateGridPosition pos_south(5.0, 5.0, cse498::Direction::South);
  auto backward_south = pos_south.GetBackwardPosition();
  CHECK(backward_south.X() == 5.0);
  CHECK(backward_south.Y() == 4.0);

  // Test West (backward is East direction)
  cse498::StateGridPosition pos_west(5.0, 5.0, cse498::Direction::West);
  auto backward_west = pos_west.GetBackwardPosition();
  CHECK(backward_west.X() == 6.0);
  CHECK(backward_west.Y() == 5.0);
}

TEST_CASE("Test StateGridPosition GetOffset", "[core]")
{
  cse498::StateGridPosition pos(5.0, 5.0, cse498::Direction::North);

  // Test positive offsets
  auto offset1 = pos.GetOffset(2.0, 3.0);
  CHECK(offset1.X() == 7.0);
  CHECK(offset1.Y() == 8.0);
  CHECK(offset1.GetDirection() == cse498::Direction::North);

  // Test negative offsets
  auto offset2 = pos.GetOffset(-1.0, -2.0);
  CHECK(offset2.X() == 4.0);
  CHECK(offset2.Y() == 3.0);

  // Test zero offset
  auto offset3 = pos.GetOffset(0.0, 0.0);
  CHECK(offset3.X() == 5.0);
  CHECK(offset3.Y() == 5.0);

  // Original should be unchanged
  CHECK(pos.X() == 5.0);
  CHECK(pos.Y() == 5.0);
}

TEST_CASE("Test StateGridPosition EuclideanDistance", "[core]")
{
  cse498::StateGridPosition pos1(0.0, 0.0);
  cse498::StateGridPosition pos2(3.0, 4.0);
  cse498::StateGridPosition pos3(0.0, 0.0);

  // Test 1
  CHECK(pos1.EuclideanDistance(pos2) == 5.0);
  CHECK(pos2.EuclideanDistance(pos1) == 5.0);

  // Test same position
  CHECK(pos1.EuclideanDistance(pos3) == 0.0);

  // Test horizontal distance
  cse498::StateGridPosition pos4(5.0, 0.0);
  CHECK(pos1.EuclideanDistance(pos4) == 5.0);

  // Test vertical distance
  cse498::StateGridPosition pos5(0.0, 7.0);
  CHECK(pos1.EuclideanDistance(pos5) == 7.0);
}

TEST_CASE("Test StateGridPosition Edge Cases - Boundary Values", "[core]")
{
  // Test at origin
  cse498::StateGridPosition origin(0.0, 0.0);
  CHECK(origin.X() == 0.0);
  CHECK(origin.Y() == 0.0);
  CHECK(origin.CellX() == 0);
  CHECK(origin.CellY() == 0);

  // Test with fractional values near boundaries
  cse498::StateGridPosition fractional(0.9, 0.1);
  CHECK(fractional.CellX() == 0);
  CHECK(fractional.CellY() == 0);

  // Test large values
  cse498::StateGridPosition large(999999.0, 888888.0);
  CHECK(large.X() == 999999.0);
  CHECK(large.Y() == 888888.0);
}

TEST_CASE("Test StateGridPosition Edge Cases - Direction Wrapping", "[core]")
{
  cse498::StateGridPosition pos(5.0, 5.0, cse498::Direction::North);

  // Test multiple rotations wrap correctly
  for (int i = 0; i < 4; ++i) {
    pos.TurnRight();
  }
  CHECK(pos.GetDirection() == cse498::Direction::North);

  for (int i = 0; i < 4; ++i) {
    pos.TurnLeft();
  }
  CHECK(pos.GetDirection() == cse498::Direction::North);

  // Test TurnAround twice returns to original
  pos.TurnAround();
  pos.TurnAround();
  CHECK(pos.GetDirection() == cse498::Direction::North);
}


// Need working StateGrid first
// These tests will most likely change based on the StateGrid methods available

//TEST_CASE("Test StateGridPosition Edge Cases - Position at Boundary", "[core]")
//{
//  // Test forward position at edge (y=0)
//  cse498::StateGridPosition edge_north(5.0, 0.0, cse498::Direction::North);
//  auto forward = edge_north.GetForwardPosition();
//  CHECK(forward.Y() == -1.0);  // Goes negative (grid validation will handle)
//
//  // Test backward position at edge
//  auto backward = edge_north.GetBackwardPosition();
//  CHECK(backward.Y() == 1.0);
//}

/*
TEST_CASE("Test StateGridPosition MoveForward", "[core]")
{
  // Mock or create a simple StateGrid for testing
  cse498::StateGrid grid(10, 10);  // 10x10 grid

  cse498::StateGridPosition pos(5, 5, cse498::Direction::North);

  // Test successful move
  bool moved = pos.MoveForward(grid);
  CHECK(moved == true);
  CHECK(pos.Y() == 4.0);
  CHECK(pos.X() == 5.0);

  // Test move into blocked cell
  // grid.SetBlocked(5, 3, true);
  // moved = pos.MoveForward(grid);
  // CHECK(moved == false);
  // CHECK(pos.Y() == 4.0);  // Position unchanged

  // Test move out of bounds
  cse498::StateGridPosition edge_pos(0, 0, cse498::Direction::North);
  moved = edge_pos.MoveForward(grid);
  CHECK(moved == false);
  CHECK(edge_pos.X() == 0.0);
  CHECK(edge_pos.Y() == 0.0);
}

TEST_CASE("Test StateGridPosition MoveBackward", "[core]")
{
  cse498::StateGrid grid(10, 10);

  cse498::StateGridPosition pos(5, 5, cse498::Direction::North);

  // Test successful backward move (North facing, move South)
  bool moved = pos.MoveBackward(grid);
  CHECK(moved == true);
  CHECK(pos.Y() == 6.0);
  CHECK(pos.X() == 5.0);
}

TEST_CASE("Test StateGridPosition IsValidForwardMove", "[core]")
{
  cse498::StateGrid grid(10, 10);

  cse498::StateGridPosition pos(5, 5, cse498::Direction::North);

  // Test valid move
  CHECK(pos.IsValidForwardMove(grid) == true);

  // Test invalid move (out of bounds)
  cse498::StateGridPosition edge_pos(0, 0, cse498::Direction::North);
  CHECK(edge_pos.IsValidForwardMove(grid) == false);

  // Verify position unchanged after check
  CHECK(pos.X() == 5.0);
  CHECK(pos.Y() == 5.0);
}

TEST_CASE("Test StateGridPosition IsValidBackwardMove", "[core]")
{
  cse498::StateGrid grid(10, 10);

  cse498::StateGridPosition pos(5, 5, cse498::Direction::North);

  // Test valid backward move
  CHECK(pos.IsValidBackwardMove(grid) == true);

  // Test invalid backward move (out of bounds)
  cse498::StateGridPosition edge_pos(9, 9, cse498::Direction::South);
  CHECK(edge_pos.IsValidBackwardMove(grid) == false);
}
*/
