/***********************************************
 * If This gets used we will likley make a ton of these virtual
 * and derive useful tiles like walls, paths, ect.
 ***********************************************/

#pragma once

#include <memory>
#include <string>

namespace cse498 {
/***********************************************
 * Forward declaration for Agent
 ***********************************************/
class Agent;

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
  int row;            // Row of the tile in the State grid, vector[row, col]
  int column;         // Column of the tile in the State grid, vector[row, col]
  char symbol;        // Simple single symbol representation
  bool canTraverse;   // Stores if you can walk on it
  std::string name;   // Name of the tile
  MetaData metadata;  // Meta data to simplify class variables

  std::shared_ptr<Agent> agent;  // Single occupant (nullptr if empty)

 public:
  Tile(int row, int column, char symbol, bool canTraverse,
       const std::string &name, const MetaData &metaData);

  /***********************************************
   *  Sets the agent pointer
   *
   * @param newAgent The agent pointer to be added
   *
   * @returns True if successful, false if it is occupied
   ***********************************************/
  bool addAgent(const std::shared_ptr<Agent> &newAgent);

  /***********************************************
   *  Removes the agent
   *
   * @returns True if successful, false if there is no agent to remove
   ***********************************************/
  bool removeAgent();

  /***********************************************
   *  Returns an Agent pointer, needs to be able to move from tile to tile
   * so i use a shared pointer
   *
   * @returns the shared pointer to the agent
   ***********************************************/
  std::shared_ptr<Agent> getAgent() const;

  /***********************************************
   *  Returns whether this tile stores an agent
   *
   * @returns bool to indicate occupancy
   ***********************************************/
  bool hasAgent() const;

  /***********************************************
   *  Returns whether the tile can be traversed
   *
   * @returns bool to indicate traversability
   ***********************************************/
  bool getCanTraverse() const;

  /***********************************************
   *  Sets traverseability, not no error checking, If its set to true
   *  and you try to se it to true anyway, this will work
   *
   * @param traversability to set the new status
   ***********************************************/
  void setCanTraverse(bool traversability);

  /***********************************************
   *  Row getter
   *
   * @returns int, row of the tile
   ***********************************************/
  int getRow() const;

  /***********************************************
   *  Col getter
   *
   * @returns int, col of the tile
   ***********************************************/
  int getColumn() const;

  /***********************************************
   *  Symbol getter
   *
   * @returns char, symbol of the tile
   ***********************************************/
  char getSymbol() const;

  /***********************************************
   *  Name getter
   *
   * @returns string ref, name of the tile
   ***********************************************/
  const std::string &getName() const;

  /***********************************************
   *  MetaData getter
   *
   * @returns MetaData ref, metadata of the tile
   ***********************************************/
  const MetaData &getMetaData() const;
};
}  // namespace cse498