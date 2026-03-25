#pragma once

#include <string>
#include <vector>
#include <cassert>

#include "Tile.hpp"

namespace cse498 {
/**********************************************
 * The grid stores Tiles in a 2D vector indexed as tiles[row][col].
 * It supports initialization from a premade symbol map and provides
 * basic utility functions such as bounds checking and metadata getters.
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
  StateGrid(const std::vector<std::vector<char>>& premadeMap);

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
   * Specific tile getter
   *
   * @param row row of the tile to get
   * @param col col of the tile to get
   *
   * @returns a pointer to the tile, null pointer if out of bounds
   ************************************************/
  [[nodiscard]] Tile* getTile(size_t row, size_t col);

  /***********************************************
   * All tile getter
   *
   * @returns a reference to the tiles vector so it's mutable
   ************************************************/
  [[nodiscard]] std::vector<std::vector<Tile>>& getAllTiles();

  /***********************************************
   * Checks whether a grid coordinate is within bounds and traversable.
   *
   * @param row Row index to check.
   * @param col Column index to check.
   *
   * @returns True if (row, col) is inside the grid and the tile is traversable.
   ************************************************/
  bool inBounds(size_t row, size_t col) const;
};
}  // namespace cse498