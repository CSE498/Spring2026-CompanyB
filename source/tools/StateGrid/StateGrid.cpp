#include "StateGrid.hpp"

#include <stdexcept>

namespace cse498 {

// Helper to move symbol table logic up here for easy access

Tile tileFromSymbol(char symbol, int r, int c) {
  if (symbol == 'W') {
    return Tile(r, c, 'W', false, "Wall", MetaData());
  } else if (symbol == 'P') {
    return Tile(r, c, 'P', true, "Path", MetaData());
  } else {
    throw std::invalid_argument(
        "Only supported symbols rn are W and P for walls, paths");
  }
}

StateGrid::StateGrid(int width, int height,
                     const std::vector<std::vector<char>> &premadeMap)
    : width(width), height(height) {
  // An LLM helped with error handling

  // Double checking the map's height and widt with the enterd height and width
  // feels like good sanity checking, validation but i might be wrong, i could
  // just change it to use the map and pull it's height and width
  if (width <= 0 || height <= 0) {
    throw std::invalid_argument("Width and height must be positive.");
  }

  if (static_cast<int>(premadeMap.size()) != height) {
    throw std::invalid_argument("premadeMap height does not match height.");
  }

  for (int r = 0; r < height; ++r) {
    if (static_cast<int>(premadeMap[r].size()) != width) {
      throw std::invalid_argument("premadeMap width does not match width.");
    }
  }

  tiles.resize(height);
  for (int r = 0; r < height; ++r) {
    tiles[r].reserve(width);
    for (int c = 0; c < width; ++c) {
      tiles[r].push_back(tileFromSymbol(premadeMap[r][c], r, c));
    }
  }
}

int StateGrid::getWidth() const { return width; }

int StateGrid::getHeight() const { return height; }

Tile *StateGrid::getTile(int row, int col) {
  if (row < 0 || row >= height || col < 0 || col >= width) {
    return nullptr;
  }

  return &tiles[row][col];
}

std::vector<std::vector<Tile>> &StateGrid::getAllTiles() { return tiles; }

bool StateGrid::inBounds(int row, int col) const {
  // inbounds check
  if (row < 0 || row >= height || col < 0 || col >= width) return false;
  // Checks for if the tile is a wall to ensure you cant walk through, on them
  return tiles[row][col].getCanTraverse();
}
}  // namespace cse498