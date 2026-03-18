#include <catch2/catch_test_macros.hpp>
#include "../../source/tools/StateGrid/StateGrid.hpp"
#include "../../source/tools/StateGrid/Tile.hpp"

namespace cse498
{
/***********************************************
 * Just to be used for testing
 ***********************************************/
class Agent
{
  public:
    explicit Agent(int id) : id(id)
    {
    }
    int getId() const
    {
        return id;
    }

  private:
    int id;
};

// AI helped with conversting test case from using Agent* to shared pointers
TEST_CASE("Test Tile")
{
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
    auto Agent1 = std::make_shared<Agent>(1);
    auto Agent2 = std::make_shared<Agent>(2);

    // Sets agent and ensure only one may exist
    CHECK(Tile1.addAgent(Agent1) == true);
    CHECK(Tile1.hasAgent() == true);

    CHECK(Tile1.addAgent(Agent2) == false);

    // Ensure that the agent is the same
    std::shared_ptr<Agent> found = Tile1.getAgent(); // FIXED: Changed to shared_ptr
    CHECK(found != nullptr);
    CHECK(found.get() == Agent1.get()); // FIXED: Compare raw pointers
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

TEST_CASE("Test StateGrid")
{
    // Basic Constructor and value checks
    std::vector<std::vector<char>> premadeMap = {{'P', 'P', 'W'}, {'P', 'W', 'P'}};

    StateGrid Stategrid1(3, 2, premadeMap);

    CHECK(Stategrid1.getWidth() == 3);
    CHECK(Stategrid1.getHeight() == 2);

    // Out of bounds check also checks for wall collisions
    CHECK(Stategrid1.inBounds(0, 0) == true);
    CHECK(Stategrid1.inBounds(1, 2) == true);
    CHECK(Stategrid1.inBounds(0, 2) == false); // wall
    CHECK(Stategrid1.inBounds(1, 1) == false); // Wall

    // All out of bounds, greater, less than height, width
    CHECK(Stategrid1.inBounds(-1, 0) == false);
    CHECK(Stategrid1.inBounds(0, -1) == false);
    CHECK(Stategrid1.inBounds(2, 0) == false);
    CHECK(Stategrid1.inBounds(0, 3) == false);
}

TEST_CASE("Test StateGrid Error Conditions")
{
    // Premade map and the height, width inputs dont match
    std::vector<std::vector<char>> badHeight = {{'P', 'P', 'W'}};
    CHECK_THROWS(StateGrid(3, 2, badHeight));

    std::vector<std::vector<char>> badWidth = {{'P', 'P'}, {'P', 'W'}};
    CHECK_THROWS(StateGrid(3, 2, badWidth));

    std::vector<std::vector<char>> tiny = {{'P'}};
    CHECK_THROWS(StateGrid(0, 1, tiny));
    CHECK_THROWS(StateGrid(1, 0, tiny));
    CHECK_THROWS(StateGrid(-1, 1, tiny));
    CHECK_THROWS(StateGrid(1, -1, tiny));
}

// AI helped with conversting test case from using Agent* to shared pointers
TEST_CASE("Test StateGrid, Tile Integration")
{
    // Innits
    std::vector<std::vector<char>> premadeMap = {{'P', 'W', 'W'}, {'P', 'W', 'P'}};

    StateGrid Stategrid1(3, 2, premadeMap);

    // Checks the tiles were initialized correctly and we can pull them
    auto &tiles = Stategrid1.getAllTiles();

    // Checks a path
    Tile &Tile_0_0 = tiles[0][0];
    CHECK(Tile_0_0.getRow() == 0);
    CHECK(Tile_0_0.getColumn() == 0);
    CHECK(Tile_0_0.getSymbol() == 'P');
    CHECK(Tile_0_0.getCanTraverse() == true);

    CHECK(Tile_0_0.getMetaData().movementModifier == 0);
    CHECK(Tile_0_0.getMetaData().condition == Condition::Perfect);

    CHECK(Tile_0_0.hasAgent() == false);
    CHECK(Tile_0_0.getAgent() == nullptr);

    // Checks a wall
    Tile &Tile_1_1 = tiles[1][1];
    CHECK(Tile_1_1.getRow() == 1);
    CHECK(Tile_1_1.getColumn() == 1);
    CHECK(Tile_1_1.getSymbol() == 'W');
    CHECK(Tile_1_1.getCanTraverse() == false);

    CHECK(Tile_1_1.getMetaData().movementModifier == 0);
    CHECK(Tile_1_1.getMetaData().condition == Condition::Perfect);

    CHECK(Tile_1_1.hasAgent() == false);
    CHECK(Tile_1_1.getAgent() == nullptr);

    // Ensures mutability of the tiles
    auto agent = std::make_shared<Agent>(42);
    CHECK(Tile_0_0.addAgent(agent) == true);
    CHECK(Tile_0_0.hasAgent() == true);
    CHECK(Tile_0_0.getAgent() == agent); // FIXED: Compare shared_ptrs directly

    // Checks pulling single tiles, getTile with coords
    Tile *Tile_0_0_again = Stategrid1.getTile(0, 0);
    CHECK(Tile_0_0_again->hasAgent() == true);
    CHECK(Tile_0_0_again->getAgent() == agent); // FIXED: Compare shared_ptrs directly

    // OOB tiles returns null ptr
    Tile *Tile_OOB = Stategrid1.getTile(5, 3);
    CHECK(Tile_OOB == nullptr);
}

TEST_CASE("Tests Moving agents")
{
    std::vector<std::vector<char>> premadeMap = {{'P', 'W', 'P'}, {'P', 'W', 'P'}};

    StateGrid Stategrid1(3, 2, premadeMap);

    auto &tiles = Stategrid1.getAllTiles();

    auto agent = std::make_shared<Agent>(42);
    CHECK(tiles[0][0].addAgent(agent) == true);
    CHECK(tiles[0][0].hasAgent() == true);
    CHECK(tiles[0][0].getAgent()->getId() == 42);

    // Get the shared_ptr before removing
    std::shared_ptr<Agent> moveAgent = tiles[0][0].getAgent();

    CHECK(tiles[0][0].removeAgent() == true);
    CHECK(tiles[0][0].hasAgent() == false);
    CHECK(tiles[0][0].getAgent() == nullptr);

    CHECK(tiles[0][1].getCanTraverse() == false); // Fixed missing parentheses

    CHECK(tiles[0][2].addAgent(moveAgent) == true);
    CHECK(tiles[0][2].hasAgent() == true);
    CHECK(tiles[0][2].getAgent()->getId() == 42);
}
}