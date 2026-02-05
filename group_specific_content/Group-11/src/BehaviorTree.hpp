#pragma once

#include "Node.hpp"

#include <memory>
#include <vector>
#include <ostream>

class BehaviorTree
{
public:
    BehaviorTree();

    void addNode(int id); // Insertion
    bool deleteNode(int id); // Deletion

    void getActivePath(); // Searching
    int tickCount() const;

    void debugView(std::ostream& out) const;

private:
    std::vector< std::unique_ptr<Node> > nodes;
    int ticks;
};