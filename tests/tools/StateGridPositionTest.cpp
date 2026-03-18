#include <catch2/catch.hpp>
#include "../../source/tools/StateGridPosition.hpp"
#include "../../source/tools/StateGrid/StateGrid.hpp"

TEST_CASE("Test StateGridPosition Constructors", "[core]")
{
  cse498::StateGridPosition pos1;  // Default constructor
  cse498::StateGridPosition pos2(5, 10);  // size_t constructor
  cse498::StateGridPosition pos3(3, 7);  // Template constructor
  cse498::StateGridPosition pos4(5, 10, cse498::Direction::East);  // With direction
  cse498::StateGridPosition pos5(pos2);  // Copy constructor

  // Test default values
  CHECK(pos1.Col() == 0);
  CHECK(pos1.Row() == 0);
  CHECK(pos1.GetDirection() == cse498::Direction::North);

  // Test size_t constructor
  CHECK(pos2.Col() == 5);
  CHECK(pos2.Row() == 10);
  CHECK(pos2.GetDirection() == cse498::Direction::North);

  // Test Template constructor
  CHECK(pos3.Col() == 3);
  CHECK(pos3.Row() == 7);

  // Test constructor with direction
  CHECK(pos4.GetDirection() == cse498::Direction::East);

  // Test copy constructor
  CHECK(pos5.Col() == pos2.Col());
  CHECK(pos5.Row() == pos2.Row());
  CHECK(pos5.GetDirection() == pos2.GetDirection());
}

TEST_CASE("Test StateGridPosition Assignment", "[core]")
{
  cse498::StateGridPosition pos1(3, 4, cse498::Direction::South);
  cse498::StateGridPosition pos2;

  pos2 = pos1;  // Assignment operator

  CHECK(pos2.Col() == 3);
  CHECK(pos2.Row() == 4);
  CHECK(pos2.GetDirection() == cse498::Direction::South);
}

TEST_CASE("Test StateGridPosition Accessors", "[core]")
{
  cse498::StateGridPosition pos(7, 9, cse498::Direction::West);

  // Test coordinate accessors
  CHECK(pos.Col() == 7);
  CHECK(pos.Row() == 9);

  // Test direction accessor
  CHECK(pos.GetDirection() == cse498::Direction::West);

  // Test with large values
  cse498::StateGridPosition pos2(1000, 2000);
  CHECK(pos2.Col() == 1000);
  CHECK(pos2.Row() == 2000);
}

TEST_CASE("Test StateGridPosition Comparisons", "[core]")
{
  cse498::StateGridPosition pos1(5, 10, cse498::Direction::North);
  cse498::StateGridPosition pos2(5, 10, cse498::Direction::North);
  cse498::StateGridPosition pos3(5, 10, cse498::Direction::East);
  cse498::StateGridPosition pos4(3, 4, cse498::Direction::North);

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

}

TEST_CASE("Test StateGridPosition GetForwardPosition", "[core]")
{
  // Test North
  cse498::StateGridPosition pos_north(5, 5, cse498::Direction::North);
  auto forward_north = pos_north.GetForwardPosition();
  CHECK(forward_north.Col() == 5);
  CHECK(forward_north.Row() == 4);
  CHECK(forward_north.GetDirection() == cse498::Direction::North);

  // Test East
  cse498::StateGridPosition pos_east(5, 5, cse498::Direction::East);
  auto forward_east = pos_east.GetForwardPosition();
  CHECK(forward_east.Col() == 6);
  CHECK(forward_east.Row() == 5);

  // Test South
  cse498::StateGridPosition pos_south(5, 5, cse498::Direction::South);
  auto forward_south = pos_south.GetForwardPosition();
  CHECK(forward_south.Col() == 5);
  CHECK(forward_south.Row() == 6);

  // Test West
  cse498::StateGridPosition pos_west(5, 5, cse498::Direction::West);
  auto forward_west = pos_west.GetForwardPosition();
  CHECK(forward_west.Col() == 4);
  CHECK(forward_west.Row() == 5);

  // Verify original position unchanged
  CHECK(pos_north.Col() == 5);
  CHECK(pos_north.Row() == 5);
}

TEST_CASE("Test StateGridPosition GetBackwardPosition", "[core]")
{
  // Test North (backward is South direction)
  cse498::StateGridPosition pos_north(5, 5, cse498::Direction::North);
  auto backward_north = pos_north.GetBackwardPosition();
  CHECK(backward_north.Col() == 5);
  CHECK(backward_north.Row() == 6);

  // Test East (backward is West direction)
  cse498::StateGridPosition pos_east(5, 5, cse498::Direction::East);
  auto backward_east = pos_east.GetBackwardPosition();
  CHECK(backward_east.Col() == 4);
  CHECK(backward_east.Row() == 5);

  // Test South (backward is North direction)
  cse498::StateGridPosition pos_south(5, 5, cse498::Direction::South);
  auto backward_south = pos_south.GetBackwardPosition();
  CHECK(backward_south.Col() == 5);
  CHECK(backward_south.Row() == 4);

  // Test West (backward is East direction)
  cse498::StateGridPosition pos_west(5, 5, cse498::Direction::West);
  auto backward_west = pos_west.GetBackwardPosition();
  CHECK(backward_west.Col() == 6);
  CHECK(backward_west.Row() == 5);
}

TEST_CASE("Test StateGridPosition GetOffset", "[core]")
{
  cse498::StateGridPosition pos(5, 5, cse498::Direction::North);

  // Test positive offsets
  auto offset1 = pos.GetOffset(2LL, 3LL);
  CHECK(offset1.Col() == 7);
  CHECK(offset1.Row() == 8);
  CHECK(offset1.GetDirection() == cse498::Direction::North);

  // Test negative offsets
  auto offset2 = pos.GetOffset(-1LL, -2LL);
  CHECK(offset2.Col() == 4);
  CHECK(offset2.Row() == 3);

  // Test zero offset
  auto offset3 = pos.GetOffset(0LL, 0LL);
  CHECK(offset3.Col() == 5);
  CHECK(offset3.Row() == 5);

  // Original should be unchanged
  CHECK(pos.Col() == 5);
  CHECK(pos.Row() == 5);
}

TEST_CASE("Test StateGridPosition EuclideanDistance", "[core]")
{
  cse498::StateGridPosition pos1(0, 0);
  cse498::StateGridPosition pos2(3, 4);
  cse498::StateGridPosition pos3(0, 0);

  // Test 1
  CHECK(pos1.EuclideanDistance(pos2) == 5.0);
  CHECK(pos2.EuclideanDistance(pos1) == 5.0);

  // Test same position
  CHECK(pos1.EuclideanDistance(pos3) == 0.0);

  // Test horizontal distance
  cse498::StateGridPosition pos4(5, 0);
  CHECK(pos1.EuclideanDistance(pos4) == 5.0);

  // Test vertical distance
  cse498::StateGridPosition pos5(0, 7);
  CHECK(pos1.EuclideanDistance(pos5) == 7.0);
}

TEST_CASE("Test StateGridPosition Edge Cases - Boundary Values", "[core]")
{
  // Test at origin
  cse498::StateGridPosition origin(0, 0);
  CHECK(origin.Col() == 0);
  CHECK(origin.Row() == 0);

  // Test large values
  cse498::StateGridPosition large(999999, 888888);
  CHECK(large.Col() == 999999);
  CHECK(large.Row() == 888888);
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

  // Moving North decreases Row by 1
  cse498::StateGridPosition pos(2, 2, cse498::Direction::North);
  CHECK(pos.MoveForward(grid) == true);
  CHECK(pos.Col() == 2);
  CHECK(pos.Row() == 1);

  // Moving East increases Col by 1
  cse498::StateGridPosition pos2(2, 2, cse498::Direction::East);
  CHECK(pos2.MoveForward(grid) == true);
  CHECK(pos2.Col() == 3);
  CHECK(pos2.Row() == 2);

  // Moving South increases Row
  cse498::StateGridPosition pos3(2, 2, cse498::Direction::South);
  CHECK(pos3.MoveForward(grid) == true);
  CHECK(pos3.Col() == 2);
  CHECK(pos3.Row() == 3);

  // Moving West decreases Col
  cse498::StateGridPosition pos4(2, 2, cse498::Direction::West);
  CHECK(pos4.MoveForward(grid) == true);
  CHECK(pos4.Col() == 1);
  CHECK(pos4.Row() == 2);

  // Attempting to move out of bounds should return false and not change position
  cse498::StateGridPosition pos5(0, 0, cse498::Direction::North);
  CHECK(pos5.MoveForward(grid) == false);
  CHECK(pos5.Col() == 0);
  CHECK(pos5.Row() == 0);

  // A map with a wall
  std::vector<std::vector<char>> wall_map = {
    {'P', 'P', 'P'},
    {'P', 'W', 'P'},
    {'P', 'P', 'P'}
  };
  // Should not be able to move into a space with a wall
  cse498::StateGrid walled_grid(3, 3, wall_map);
  cse498::StateGridPosition pos6(1, 0, cse498::Direction::South);
  CHECK(pos6.MoveForward(walled_grid) == false);
  CHECK(pos6.Col() == 1);
  CHECK(pos6.Row() == 0);
}

TEST_CASE("Test StateGridPosition MoveBackward", "[core]")
{
  auto map = createOpenMap();
  cse498::StateGrid grid(5, 5, map);

  // Moving south should increase Row by 1
  cse498::StateGridPosition pos(2, 2, cse498::Direction::North);
  CHECK(pos.MoveBackward(grid) == true);
  CHECK(pos.Col() == 2);
  CHECK(pos.Row() == 3);

  // Moving north should decrease Row by 1
  cse498::StateGridPosition pos2(2, 2, cse498::Direction::South);
  CHECK(pos2.MoveBackward(grid) == true);
  CHECK(pos2.Col() == 2);
  CHECK(pos2.Row() == 1);

  // Attempting to move out of bounds should fail
  cse498::StateGridPosition pos3(4, 4, cse498::Direction::North);
  CHECK(pos3.MoveBackward(grid) == false);
  CHECK(pos3.Col() == 4);
  CHECK(pos3.Row() == 4);
}

TEST_CASE("Test StateGridPosition IsValidForwardMove", "[core]")
{
  auto map = createOpenMap();
  cse498::StateGrid grid(5, 5, map);

  // Move is valid, but shouldn't change position
  cse498::StateGridPosition pos(2, 2, cse498::Direction::North);
  CHECK(pos.IsValidForwardMove(grid) == true);
  CHECK(pos.Col() == 2);
  CHECK(pos.Row() == 2);

  // Move isn't valid, so should return false
  cse498::StateGridPosition pos2(0, 0, cse498::Direction::North);
  CHECK(pos2.IsValidForwardMove(grid) == false);

  // Another map with a wall
  std::vector<std::vector<char>> wall_map = {
    {'P', 'W'},
    {'P', 'P'}
  };
  // Should return false for inquiring about moving into a spot with a wall
  cse498::StateGrid walled_grid(2, 2, wall_map);
  cse498::StateGridPosition pos3(0, 0, cse498::Direction::East);
  CHECK(pos3.IsValidForwardMove(walled_grid) == false);
}

TEST_CASE("Test StateGridPosition IsValidBackwardMove", "[core]")
{
  auto map = createOpenMap();
  cse498::StateGrid grid(5, 5, map);

  // Move is valid, but shouldn't change position
  cse498::StateGridPosition pos(2, 2, cse498::Direction::North);
  CHECK(pos.IsValidBackwardMove(grid) == true);
  CHECK(pos.Col() == 2);
  CHECK(pos.Row() == 2);

  // Move isn't valid, so should return false
  cse498::StateGridPosition pos2(2, 4, cse498::Direction::North);
  CHECK(pos2.IsValidBackwardMove(grid) == false);
}
