#include <catch2/catch_test_macros.hpp>

#include "../../source/tools/StateGrid/StateGrid.hpp"
#include "../../source/tools/StateGrid/Tile.hpp"

namespace cse498 {
// AI helped with conversting test case from using Agent* to shared pointers
TEST_CASE("Test Tile") {
  // Setting new values
  MetaData MetaData1;
  MetaData1.movementModifier = 2;
  MetaData1.condition = Condition::Wet;

  // Defualt values
  MetaData MetaData2;

  // Initializing tiles
  Tile Tile1(0, 0, 'P', true, "Path", MetaData1);
  Tile Tile2(1, 2, 'W', false, "Wall", MetaData2);

  // making sure that everything was initialized and set correctly
  CHECK(Tile1.getRow() == 0);
  CHECK(Tile1.getColumn() == 0);
  CHECK(Tile1.getSymbol() == 'P');
  CHECK(Tile1.getCanTraverse() == true);
  CHECK(Tile1.getName() == "Path");
  CHECK(Tile1.getMetaData().movementModifier == 2);
  CHECK(Tile1.getMetaData().condition == Condition::Wet);

  CHECK(Tile2.getRow() == 1);
  CHECK(Tile2.getColumn() == 2);
  CHECK(Tile2.getSymbol() == 'W');
  CHECK(Tile2.getCanTraverse() == false);
  CHECK(Tile2.getName() == "Wall");
  CHECK(Tile2.getMetaData().movementModifier == 0);
  CHECK(Tile2.getMetaData().condition == Condition::Perfect);

  // Chcking Agent handling, no agents
  CHECK(Tile1.getAgent() == nullptr);
  CHECK(Tile1.hasAgent() == false);

  // Default innits
  auto Agent1 = std::make_unique<Agent>(1);
  auto Agent2 = std::make_unique<Agent>(2);

  // Save raw pointer before moving ownership
  Agent *Agent1_raw = Agent1.get();

  // Sets agent and ensure only one may exist
  CHECK(Tile1.addAgent(Agent1) == true);
  CHECK(Tile1.hasAgent() == true);
  CHECK(Agent1 == nullptr);  // ownership was transferred

  CHECK(Tile1.addAgent(Agent2) == false);
  CHECK(Agent2 != nullptr);  // caller retains ownership on failure
  CHECK(Agent2->getId() == 2);

  // Ensure that the agent is the same
  Agent *found = Tile1.getAgent();
  CHECK(found != nullptr);
  CHECK(found == Agent1_raw);
  CHECK(found->getId() == 1);

  // Removes agent and esure you can remove an agent if there isnt one
  CHECK(Tile1.removeAgent() == true);
  CHECK(Tile1.hasAgent() == false);
  CHECK(Tile1.getAgent() == nullptr);

  CHECK(Tile1.removeAgent() == false);

  // Checks traversability
  CHECK(Tile1.getCanTraverse() == true);
  Tile1.setCanTraverse(false);
  CHECK(Tile1.getCanTraverse() == false);
}

TEST_CASE("Test StateGrid") {
  // Basic Constructor and value checks
  std::vector<std::vector<char>> premadeMap = {{'P', 'P', 'W'},
                                               {'P', 'W', 'P'}};

  StateGrid Stategrid1(premadeMap);

  CHECK(Stategrid1.getWidth() == 3);
  CHECK(Stategrid1.getHeight() == 2);

  // Out of bounds check also checks for wall collisions
  CHECK(Stategrid1.inBounds(0, 0) == true);
  CHECK(Stategrid1.inBounds(1, 2) == true);
  CHECK(Stategrid1.inBounds(0, 2) == false);  // wall
  CHECK(Stategrid1.inBounds(1, 1) == false);  // Wall

  // All out of bounds, greater, less than height, width
  CHECK(Stategrid1.inBounds(-1, 0) == false);
  CHECK(Stategrid1.inBounds(0, -1) == false);
  CHECK(Stategrid1.inBounds(2, 0) == false);
  CHECK(Stategrid1.inBounds(0, 3) == false);
}

// AI helped with conversting test case from using Agent* to shared pointers
TEST_CASE("Test StateGrid, Tile Integration") {
  // Innits
  std::vector<std::vector<char>> premadeMap = {{'P', 'W', 'W'},
                                               {'P', 'W', 'P'}};

  StateGrid Stategrid1(premadeMap);

  // Checks the tiles were initialized correctly using StateGrid accessors
  // Checks a path
  CHECK(Stategrid1.getSymbol(0, 0) == 'P');
  CHECK(Stategrid1.getCanTraverse(0, 0) == true);
  CHECK(Stategrid1.getMetaData(0, 0).movementModifier == 0);
  CHECK(Stategrid1.getMetaData(0, 0).condition == Condition::Perfect);
  CHECK(Stategrid1.hasAgent(0, 0) == false);
  CHECK(Stategrid1.getAgent(0, 0) == nullptr);

  // Checks a wall
  CHECK(Stategrid1.getSymbol(1, 1) == 'W');
  CHECK(Stategrid1.getCanTraverse(1, 1) == false);
  CHECK(Stategrid1.getMetaData(1, 1).movementModifier == 0);
  CHECK(Stategrid1.getMetaData(1, 1).condition == Condition::Perfect);
  CHECK(Stategrid1.hasAgent(1, 1) == false);
  CHECK(Stategrid1.getAgent(1, 1) == nullptr);

  // Also verify via const getAllTiles
  const auto &tiles = Stategrid1.getAllTiles();
  CHECK(tiles[0][0].getRow() == 0);
  CHECK(tiles[0][0].getColumn() == 0);
  CHECK(tiles[1][1].getRow() == 1);
  CHECK(tiles[1][1].getColumn() == 1);

  // Ensures mutability through StateGrid wrapper methods
  auto agent = std::make_unique<Agent>(42);
  Agent *agent_raw = agent.get();
  CHECK(Stategrid1.addAgent(0, 0, agent) == true);
  CHECK(Stategrid1.hasAgent(0, 0) == true);
  CHECK(Stategrid1.getAgent(0, 0) == agent_raw);

  // Checks pulling single tiles, getTile with coords (now const)
  const Tile *Tile_0_0_again = Stategrid1.getTile(0, 0);
  CHECK(Tile_0_0_again->hasAgent() == true);
  CHECK(Tile_0_0_again->getAgent() == agent_raw);

  // OOB tiles returns null ptr
  const Tile *Tile_OOB = Stategrid1.getTile(5, 3);
  CHECK(Tile_OOB == nullptr);
}

TEST_CASE("Tests Moving agents") {
  std::vector<std::vector<char>> premadeMap = {{'P', 'W', 'P'},
                                               {'P', 'W', 'P'}};

  StateGrid Stategrid1(premadeMap);

  auto agent = std::make_unique<Agent>(42);
  CHECK(Stategrid1.addAgent(0, 0, agent) == true);
  CHECK(Stategrid1.hasAgent(0, 0) == true);
  CHECK(Stategrid1.getAgent(0, 0)->getId() == 42);

  CHECK(Stategrid1.getCanTraverse(0, 1) == false);

  // Move agent from (0,0) to (0,2) using StateGrid moveAgent
  CHECK(Stategrid1.moveAgent(0, 0, 0, 2) == true);
  CHECK(Stategrid1.hasAgent(0, 0) == false);
  CHECK(Stategrid1.getAgent(0, 0) == nullptr);
  CHECK(Stategrid1.hasAgent(0, 2) == true);
  CHECK(Stategrid1.getAgent(0, 2)->getId() == 42);

  // Try to move to an occupied tile (should fail)
  auto agent2 = std::make_unique<Agent>(99);
  CHECK(Stategrid1.addAgent(0, 0, agent2) == true);
  CHECK(Stategrid1.moveAgent(0, 0, 0, 2) == false);
  // Agent should still be on source tile after failed move
  CHECK(Stategrid1.hasAgent(0, 0) == true);
  CHECK(Stategrid1.getAgent(0, 0)->getId() == 99);

  // Remove agent
  CHECK(Stategrid1.removeAgent(0, 2) == true);
  CHECK(Stategrid1.hasAgent(0, 2) == false);
  CHECK(Stategrid1.removeAgent(0, 2) == false);  // Already empty
}
}  // namespace cse498