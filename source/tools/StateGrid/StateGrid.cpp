#include "StateGrid.hpp"


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

StateGrid::StateGrid(const std::vector<std::vector<char>>& premadeMap)
{
  // An LLM helped with error handling

  // Double checking the map's height and widt with the enterd height and width
  // feels like good sanity checking, validation but i might be wrong, i could
  // just change it to use the map and pull it's height and width

  assert(premadeMap.size() > 0 && premadeMap[0].size() > 0 && "Map not empty");

  tiles.resize(premadeMap.size());
  for (int r = 0; r < premadeMap.size(); ++r) {
    tiles[r].reserve(premadeMap[0].size());
    for (int c = 0; c < premadeMap[0].size(); ++c) {
      tiles[r].push_back(tileFromSymbol(premadeMap[r][c], r, c));
    }
  }
}

[[nodiscard]] size_t StateGrid::getWidth() const { return tiles[0].size(); }

[[nodiscard]] size_t StateGrid::getHeight() const { return tiles.size(); }

[[nodiscard]] Tile* StateGrid::getTile(size_t row, size_t col) {
  if (row >= getHeight() || col >= getWidth()) {
    return nullptr;
  }

  return &tiles[row][col];
}

[[nodiscard]] std::vector<std::vector<Tile>>& StateGrid::getAllTiles() { return tiles; }

bool StateGrid::inBounds(size_t row, size_t col) const {
  // inbounds check
  if (row >= getHeight() || col >= getWidth()) return false;
  // Checks for if the tile is a wall to ensure you cant walk through, on them
  return tiles[row][col].getCanTraverse();
}
}  // namespace cse498