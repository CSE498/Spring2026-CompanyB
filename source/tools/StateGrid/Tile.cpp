#include "Tile.hpp"

namespace cse498 {
Tile::Tile(size_t row, size_t column, char symbol, bool canTraverse,
           const std::string& name, const MetaData& metaData)
    : row(row),
      column(column),
      symbol(symbol),
      canTraverse(canTraverse),
      name(name),
      metadata(metaData),
      agent(nullptr) {}

bool Tile::addAgent(const std::shared_ptr<Agent>& newAgent) {
  // Reject null
  assert(newAgent);

  // Reject if occupied
  if (agent) return false;

  agent = newAgent;
  return true;
}

bool Tile::removeAgent() {
  if (!agent) return false;
  agent.reset();
  return true;
}

[[nodiscard]] std::shared_ptr<Agent> Tile::getAgent() const { return agent; }

bool Tile::hasAgent() const { return static_cast<bool>(agent); }

[[nodiscard]] bool Tile::getCanTraverse() const { return canTraverse; }

void Tile::setCanTraverse(bool traversability) { canTraverse = traversability; }

[[nodiscard]] size_t Tile::getRow() const { return row; }

[[nodiscard]] size_t Tile::getColumn() const { return column; }

[[nodiscard]] char Tile::getSymbol() const { return symbol; }

[[nodiscard]] const std::string& Tile::getName() const { return name; }

[[nodiscard]] const MetaData& Tile::getMetaData() const { return metadata; }
}  // namespace cse498