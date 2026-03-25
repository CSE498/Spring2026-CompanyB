#include "Tile.hpp"

namespace cse498
{
Tile::Tile(size_t row, size_t column, char symbol, bool canTraverse, const std::string &name, const MetaData &metaData)
    : row(row), column(column), symbol(symbol), canTraverse(canTraverse), name(name), metadata(metaData), agent(nullptr)
{
}

[[nodiscard]] bool Tile::addAgent(std::unique_ptr<Agent> &newAgent)
{
    // Reject null
    assert(newAgent);

    // Reject if occupied, caller keeps ownership
    if (agent)
        return false;

    agent = std::move(newAgent);
    return true;
}

[[nodiscard]] bool Tile::removeAgent()
{
    if (!agent)
        return false;
    agent.reset();
    return true;
}

std::unique_ptr<Agent> Tile::releaseAgent()
{
    return std::move(agent);
}

[[nodiscard]] Agent *Tile::getAgent() const
{
    return agent.get();
}

[[nodiscard]] bool Tile::hasAgent() const
{
    return static_cast<bool>(agent);
}


void Tile::setCanTraverse(bool traversability)
{
    canTraverse = traversability;
}


[[nodiscard]] const std::string &Tile::getName() const
{
    return name;
}

[[nodiscard]] const MetaData &Tile::getMetaData() const
{
    return metadata;
}
} // namespace cse498
