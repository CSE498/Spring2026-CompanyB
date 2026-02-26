#include "catch2/catch.hpp"
#include "../../source/tools/StateGridPosition.hpp"
#include "../../source/tools/StateGrid/StateGrid.hpp"

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

// Builds completely open 5x5 map
// This will be given to a StateGrid constuctor to build a StateGrid object
static std::vector<std::vector<char>> createOpenMap() {
  return std::vector<std::vector<char>>(5, std::vector<char>(5, 'P'));
}

TEST_CASE("Test StateGridPosition MoveForward", "[core]")
{
  auto map = createOpenMap();
  cse498::StateGrid grid(5, 5, map);

  // Moving North decreases Y by 1
  cse498::StateGridPosition pos(2.0, 2.0, cse498::Direction::North);
  CHECK(pos.MoveForward(grid) == true);
  CHECK(pos.X() == 2.0);
  CHECK(pos.Y() == 1.0);

  // Moving East increases X by 1
  cse498::StateGridPosition pos2(2.0, 2.0, cse498::Direction::East);
  CHECK(pos2.MoveForward(grid) == true);
  CHECK(pos2.X() == 3.0);
  CHECK(pos2.Y() == 2.0);

  // Moving South increases Y
  cse498::StateGridPosition pos3(2.0, 2.0, cse498::Direction::South);
  CHECK(pos3.MoveForward(grid) == true);
  CHECK(pos3.X() == 2.0);
  CHECK(pos3.Y() == 3.0);

  // Moving West decreases X
  cse498::StateGridPosition pos4(2.0, 2.0, cse498::Direction::West);
  CHECK(pos4.MoveForward(grid) == true);
  CHECK(pos4.X() == 1.0);
  CHECK(pos4.Y() == 2.0);

  // Attempting to move out of bounds should return false and not change position
  cse498::StateGridPosition pos5(0.0, 0.0, cse498::Direction::North);
  CHECK(pos5.MoveForward(grid) == false);
  CHECK(pos5.X() == 0.0);
  CHECK(pos5.Y() == 0.0);

  // A map with a wall
  std::vector<std::vector<char>> wall_map = {
    {'P', 'P', 'P'},
    {'P', 'W', 'P'},
    {'P', 'P', 'P'}
  };
  // Should not be able to move into a space with a wall
  cse498::StateGrid walled_grid(3, 3, wall_map);
  cse498::StateGridPosition pos6(1.0, 0.0, cse498::Direction::South);
  CHECK(pos6.MoveForward(walled_grid) == false);
  CHECK(pos6.X() == 1.0);
  CHECK(pos6.Y() == 0.0);
}

TEST_CASE("Test StateGridPosition MoveBackward", "[core]")
{
  auto map = createOpenMap();
  cse498::StateGrid grid(5, 5, map);

  // Moving south should increase Y by 1
  cse498::StateGridPosition pos(2.0, 2.0, cse498::Direction::North);
  CHECK(pos.MoveBackward(grid) == true);
  CHECK(pos.X() == 2.0);
  CHECK(pos.Y() == 3.0);

  // Moving north should decrease Y by 1
  cse498::StateGridPosition pos2(2.0, 2.0, cse498::Direction::South);
  CHECK(pos2.MoveBackward(grid) == true);
  CHECK(pos2.X() == 2.0);
  CHECK(pos2.Y() == 1.0);

  // Attempting to move out of bounds should fail
  cse498::StateGridPosition pos3(4.0, 4.0, cse498::Direction::North);
  CHECK(pos3.MoveBackward(grid) == false);
  CHECK(pos3.X() == 4.0);
  CHECK(pos3.Y() == 4.0);
}

TEST_CASE("Test StateGridPosition IsValidForwardMove", "[core]")
{
  auto map = createOpenMap();
  cse498::StateGrid grid(5, 5, map);

  // Move is valid, but shouldn't change position
  cse498::StateGridPosition pos(2.0, 2.0, cse498::Direction::North);
  CHECK(pos.IsValidForwardMove(grid) == true);
  CHECK(pos.X() == 2.0);
  CHECK(pos.Y() == 2.0);

  // Move isn't valid, so should return false
  cse498::StateGridPosition pos2(0.0, 0.0, cse498::Direction::North);
  CHECK(pos2.IsValidForwardMove(grid) == false);

  // Another map with a wall
  std::vector<std::vector<char>> wall_map = {
    {'P', 'W'},
    {'P', 'P'}
  };
  // Should return false for inquiring about moving into a spot with a wall
  cse498::StateGrid walled_grid(2, 2, wall_map);
  cse498::StateGridPosition pos3(0.0, 0.0, cse498::Direction::East);
  CHECK(pos3.IsValidForwardMove(walled_grid) == false);
}

TEST_CASE("Test StateGridPosition IsValidBackwardMove", "[core]")
{
  auto map = createOpenMap();
  cse498::StateGrid grid(5, 5, map);

  // Move is valid, but shouldn't change position
  cse498::StateGridPosition pos(2.0, 2.0, cse498::Direction::North);
  CHECK(pos.IsValidBackwardMove(grid) == true);
  CHECK(pos.X() == 2.0);
  CHECK(pos.Y() == 2.0);

  // Move isn't valid, so should return false
  cse498::StateGridPosition pos2(2.0, 4.0, cse498::Direction::North);
  CHECK(pos2.IsValidBackwardMove(grid) == false);
}
