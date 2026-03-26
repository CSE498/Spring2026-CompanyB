#pragma once

#include <cassert>
#include <string>
#include <vector>

#include "Tile.hpp"

namespace cse498 {
/**********************************************
 * The grid stores Tiles in a 2D vector indexed as tiles[row][col].
 * It supports initialization from a premade symbol map and provides
 * basic utility functions such as bounds checking and metadata getters.
 *
 * All tile mutation goes through StateGrid methods. Direct tile access
 * is read-only via const pointers and const references.
 **********************************************/
class StateGrid {
 private:
  std::vector<std::vector<Tile>> tiles;  // 2D storage for tiles

 public:
  /***********************************************
   * Constructs a grid using a premadeMap of symbols.
   *
   * @param premadeMap A 2D grid of characters describing the initial tile
   *symbols.
   ***********************************************/
  StateGrid(const std::vector<std::vector<char>> &premadeMap);

  // ==================== Grid Info ====================

  /***********************************************
   * Width getter
   *
   * @returns the width of the 2d tiles vector
   ************************************************/
  [[nodiscard]] size_t getWidth() const;

  /***********************************************
   * Height getter
   *
   * @returns the height of the 2d tiles vector
   ************************************************/
  [[nodiscard]] size_t getHeight() const;

  /***********************************************
   * Checks whether a grid coordinate is within bounds and traversable.
   *
   * @param row Row index to check.
   * @param col Column index to check.
   *
   * @returns True if (row, col) is inside the grid and the tile is traversable.
   ************************************************/
  [[nodiscard]] bool inBounds(size_t row, size_t col) const;

  // ==================== Tile Access (read-only) ====================

  /***********************************************
   * Specific tile getter (const)
   *
   * @param row row of the tile to get
   * @param col col of the tile to get
   *
   * @returns a const pointer to the tile, nullptr if out of bounds
   ************************************************/
  [[nodiscard]] const Tile *getTile(size_t row, size_t col) const;

  /***********************************************
   * All tile getter (const)
   *
   * @returns a const reference to the tiles vector
   ************************************************/
  [[nodiscard]] const std::vector<std::vector<Tile>> &getAllTiles() const;

  // ==================== Tile Property Access ====================

  /***********************************************
   * Gets the symbol of a tile
   *
   * @param row row of the tile
   * @param col col of the tile
   *
   * @returns char symbol of the tile
   ************************************************/
  [[nodiscard]] char getSymbol(size_t row, size_t col) const;

  /***********************************************
   * Gets the name of a tile
   *
   * @param row row of the tile
   * @param col col of the tile
   *
   * @returns const string ref, name of the tile
   ************************************************/
  [[nodiscard]] const std::string &getName(size_t row, size_t col) const;

  /***********************************************
   * Gets the metadata of a tile
   *
   * @param row row of the tile
   * @param col col of the tile
   *
   * @returns const MetaData ref
   ************************************************/
  [[nodiscard]] const MetaData &getMetaData(size_t row, size_t col) const;

  /***********************************************
   * Gets whether a tile can be traversed
   *
   * @param row row of the tile
   * @param col col of the tile
   *
   * @returns bool indicating traversability
   ************************************************/
  [[nodiscard]] bool getCanTraverse(size_t row, size_t col) const;

  // ==================== Tile Property Mutation ====================

  /***********************************************
   * Sets the traversability of a tile
   *
   * @param row row of the tile
   * @param col col of the tile
   * @param traversable new traversability value
   ************************************************/
  void setCanTraverse(size_t row, size_t col, bool traversable);

  // ==================== Agent Management ====================

  /***********************************************
   * Adds an agent to a tile. Only moves the pointer on success,
   * so the caller retains ownership if the tile is occupied.
   *
   * @param row row of the tile
   * @param col col of the tile
   * @param agent unique_ptr ref to the agent to place
   *
   * @returns true if successful, false if the tile is already occupied
   ************************************************/
  bool addAgent(size_t row, size_t col, std::unique_ptr<Agent> &agent);

  /***********************************************
   * Removes and destroys the agent on a tile
   *
   * @param row row of the tile
   * @param col col of the tile
   *
   * @returns true if an agent was removed, false if tile was empty
   ************************************************/
  bool removeAgent(size_t row, size_t col);

  /***********************************************
   * Moves an agent from one tile to another
   *
   * @param fromRow row of the source tile
   * @param fromCol col of the source tile
   * @param toRow row of the destination tile
   * @param toCol col of the destination tile
   *
   * @returns true if the move succeeded, false if source has no agent
   *          or destination is occupied
   ************************************************/
  bool moveAgent(size_t fromRow, size_t fromCol, size_t toRow, size_t toCol);

  /***********************************************
   * Gets a non-owning pointer to the agent on a tile
   *
   * @param row row of the tile
   * @param col col of the tile
   *
   * @returns raw pointer to the agent, nullptr if empty
   ************************************************/
  [[nodiscard]] Agent *getAgent(size_t row, size_t col) const;

  /***********************************************
   * Checks whether a tile has an agent
   *
   * @param row row of the tile
   * @param col col of the tile
   *
   * @returns true if the tile has an agent
   ************************************************/
  [[nodiscard]] bool hasAgent(size_t row, size_t col) const;

  void print();
};
}  // namespace cse498