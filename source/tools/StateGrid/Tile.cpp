#include "Tile.hpp"

Tile::Tile(int row, int column, char symbol, bool canTraverse, const std::string &name, const MetaData &metaData)
    : row(row), column(column), symbol(symbol), canTraverse(canTraverse), name(name), metadata(metaData), agent(nullptr)
{
}

bool Tile::addAgent(const std::shared_ptr<Agent> &newAgent)
{
    // Reject null
    if (!newAgent)
        return false;

    // Reject if occupied
    if (agent)
        return false;

    agent = newAgent;
    return true;
}

bool Tile::removeAgent()
{
    if (!agent)
        return false;
    agent.reset();
    return true;
}

std::shared_ptr<Agent> Tile::getAgent() const
{
    return agent;
}

bool Tile::hasAgent() const
{
    return static_cast<bool>(agent);
}

bool Tile::getCanTraverse() const
{
    return canTraverse;
}

int Tile::getRow() const
{
    return row;
}

int Tile::getColumn() const
{
    return column;
}

char Tile::getSymbol() const
{
    return symbol;
}

const std::string &Tile::getName() const
{
    return name;
}

const MetaData &Tile::getMetaData() const
{
    return metadata;
}
