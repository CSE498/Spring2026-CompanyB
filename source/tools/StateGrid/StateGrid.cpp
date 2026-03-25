#include <iostream>
#include "StateGrid.hpp"

namespace cse498
{

// Helper to move symbol table logic up here for easy access

Tile tileFromSymbol(char symbol, size_t r, size_t c)
{
    if (symbol == 'W')
    {
        return Tile(r, c, 'W', false, "Wall", MetaData());
    }
    else if (symbol == 'P')
    {
        return Tile(r, c, 'P', true, "Path", MetaData());
    }
    else
    {
        throw std::invalid_argument("Only supported symbols rn are W and P for walls, paths");
    }
}

StateGrid::StateGrid(const std::vector<std::vector<char>> &premadeMap)
{
    assert(premadeMap.size() > 0 && premadeMap[0].size() > 0 && "Map not empty");

    tiles.resize(premadeMap.size());
    for (size_t r = 0; r < premadeMap.size(); ++r)
    {
        tiles[r].reserve(premadeMap[0].size());
        for (size_t c = 0; c < premadeMap[0].size(); ++c)
        {
            tiles[r].push_back(tileFromSymbol(premadeMap[r][c], r, c));
        }
    }
}

// ==================== Grid Info ====================

[[nodiscard]] size_t StateGrid::getWidth() const
{
    assert(!tiles.empty() && !tiles[0].empty() && "Not an empty map");
    return tiles[0].size();
}

[[nodiscard]] size_t StateGrid::getHeight() const
{
    return tiles.size();
}

[[nodiscard]] bool StateGrid::inBounds(size_t row, size_t col) const
{
    if (row >= getHeight() || col >= getWidth())
        return false;
    return tiles[row][col].getCanTraverse();
}

// ==================== Tile Access (read-only) ====================

[[nodiscard]] const Tile *StateGrid::getTile(size_t row, size_t col) const
{
    if (row >= getHeight() || col >= getWidth())
    {
        return nullptr;
    }
    return &tiles[row][col];
}

[[nodiscard]] const std::vector<std::vector<Tile>> &StateGrid::getAllTiles() const
{
    return tiles;
}

// ==================== Tile Property Access ====================

[[nodiscard]] char StateGrid::getSymbol(size_t row, size_t col) const
{
    assert(row < getHeight() && col < getWidth() && "Coordinates in bounds");
    return tiles[row][col].getSymbol();
}

[[nodiscard]] const std::string &StateGrid::getName(size_t row, size_t col) const
{
    assert(row < getHeight() && col < getWidth() && "Coordinates in bounds");
    return tiles[row][col].getName();
}

[[nodiscard]] const MetaData &StateGrid::getMetaData(size_t row, size_t col) const
{
    assert(row < getHeight() && col < getWidth() && "Coordinates in bounds");
    return tiles[row][col].getMetaData();
}

[[nodiscard]] bool StateGrid::getCanTraverse(size_t row, size_t col) const
{
    assert(row < getHeight() && col < getWidth() && "Coordinates in bounds");
    return tiles[row][col].getCanTraverse();
}

// ==================== Tile Property Mutation ====================

void StateGrid::setCanTraverse(size_t row, size_t col, bool traversable)
{
    assert(row < getHeight() && col < getWidth() && "Coordinates in bounds");
    tiles[row][col].setCanTraverse(traversable);
}

// ==================== Agent Management ====================

bool StateGrid::addAgent(size_t row, size_t col, std::unique_ptr<Agent> &agent)
{
    assert(row < getHeight() && col < getWidth() && "Coordinates in bounds");
    return tiles[row][col].addAgent(agent);
}

bool StateGrid::removeAgent(size_t row, size_t col)
{
    assert(row < getHeight() && col < getWidth() && "Coordinates in bounds");
    return tiles[row][col].removeAgent();
}

bool StateGrid::moveAgent(size_t fromRow, size_t fromCol, size_t toRow, size_t toCol)
{
    assert(fromRow < getHeight() && fromCol < getWidth() && "Source coordinates in bounds");
    assert(toRow < getHeight() && toCol < getWidth() && "Destination coordinates in bounds");

    // Check preconditions before moving anything
    if (!tiles[fromRow][fromCol].hasAgent())
        return false;
    if (tiles[toRow][toCol].hasAgent())
        return false;


    std::unique_ptr<Agent> agent = tiles[fromRow][fromCol].releaseAgent();
    if (!tiles[toRow][toCol].addAgent(agent)) {
        return false;
    }
    return true;
}

[[nodiscard]] Agent *StateGrid::getAgent(size_t row, size_t col) const
{
    assert(row < getHeight() && col < getWidth() && "Coordinates in bounds");
    return tiles[row][col].getAgent();
}

[[nodiscard]] bool StateGrid::hasAgent(size_t row, size_t col) const
{
    assert(row < getHeight() && col < getWidth() && "Coordinates in bounds");
    return tiles[row][col].hasAgent();
}

void StateGrid::print()
{
    for (int i = 0; i < getHeight(); i++)
    {
        for (int j = 0; j < (getWidth()); j++)
            {
                std::cout << getTile(i, j)->getSymbol();
            }
        std::cout << std::endl;
    }
}
} // namespace cse498
