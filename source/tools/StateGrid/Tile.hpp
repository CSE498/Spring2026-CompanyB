/***********************************************
 * If This gets used we will likley make a ton of these virtual
 * and derive useful tiles like walls, paths, ect.
 ***********************************************/

#pragma once

#include <cassert>
#include <memory>
#include <string>

namespace cse498 {
/***********************************************
 * Forward declaration for Agent
 * Just to be used for testing
 ***********************************************/
class Agent {
 public:
  explicit Agent(int id) : id(id) {}
  int getId() const { return id; }

 private:
  int id;
};

/***********************************************
 *  Holds Possible weather conditions
 ***********************************************/
enum class Condition { Snowy, Wet, Damaged, Perfect };

/***********************************************
 *  Simplifies the Data storage, might be unneccecary
 ***********************************************/
struct MetaData {
  int movementModifier = 0;
  Condition condition = Condition::Perfect;
};

/***********************************************
 *  Class for the Tiles that we use to make up the map. Can store an Agent on
 *them and store a ton of meta data.
 ***********************************************/
class Tile {
 private:
  size_t row = 0;     // Row of the tile in the State grid, vector[row, col]
  size_t column = 0;  // Column of the tile in the State grid, vector[row, col]
  char symbol = ' ';  // Simple single symbol representation
  bool canTraverse = true;  // Stores if you can walk on it
  std::string name = "";    // Name of the tile
  MetaData metadata;        // Meta data to simplify class variables

  std::unique_ptr<Agent> agent;  // Single occupant (nullptr if empty)

 public:
  Tile(size_t row, size_t column, char symbol, bool canTraverse,
       const std::string &name, const MetaData &metaData);

  /***********************************************
   *  Sets the agent pointer. Only moves the pointer on success,
   *  so the caller retains ownership if the tile is occupied.
   *
   * @param newAgent The agent pointer to be added
   *
   * @returns True if successful, false if it is occupied
   ***********************************************/
  [[nodiscard]] bool addAgent(std::unique_ptr<Agent> &newAgent);

  /***********************************************
   *  Removes the agent
   *
   * @returns True if successful, false if there is no agent to remove
   ***********************************************/
  [[nodiscard]] bool removeAgent();

  /***********************************************
   *  Releases and returns ownership of the agent
   *
   * @returns unique_ptr to the agent, nullptr if empty
   ***********************************************/
  [[nodiscard]] std::unique_ptr<Agent> releaseAgent();

  /***********************************************
   *  Returns a non-owning pointer to the agent on this tile
   *
   * @returns raw pointer to the agent, nullptr if empty
   ***********************************************/
  [[nodiscard]] Agent *getAgent() const;

  /***********************************************
   *  Returns whether this tile stores an agent
   *
   * @returns bool to indicate occupancy
   ***********************************************/
  [[nodiscard]] bool hasAgent() const;

  /***********************************************
   *  Returns whether the tile can be traversed
   *
   * @returns bool to indicate traversability
   ***********************************************/
  [[nodiscard]] constexpr bool getCanTraverse() const { return canTraverse; };

  /***********************************************
   *  Sets whether the tile can be traversed.
   *
   * @param traversability to set the new status
   ***********************************************/
  void setCanTraverse(bool traversability);

  /***********************************************
   *  Row getter
   *
   * @returns int, row of the tile
   ***********************************************/
  [[nodiscard]] constexpr size_t getRow() const { return row; };

  /***********************************************
   *  Col getter
   *
   * @returns int, col of the tile
   ***********************************************/
  [[nodiscard]] constexpr size_t getColumn() const { return column; };

  /***********************************************
   *  Symbol getter
   *
   * @returns char, symbol of the tile
   ***********************************************/
  [[nodiscard]] constexpr char getSymbol() const { return symbol; };

  /***********************************************
   *  Name getter
   *
   * @returns string ref, name of the tile
   ***********************************************/
  [[nodiscard]] const std::string &getName() const;

  /***********************************************
   *  MetaData getter
   *
   * @returns MetaData ref, metadata of the tile
   ***********************************************/
  [[nodiscard]] const MetaData &getMetaData() const;
};
}  // namespace cse498
