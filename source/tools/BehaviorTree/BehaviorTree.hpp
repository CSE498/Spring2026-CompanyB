#pragma once

#include "SequenceNode.hpp"
#include "SelectNode.hpp"
#include "InvertNode.hpp"
#include "RepeatNode.hpp"
#include "ActionNode.hpp"

#include <memory>

class BehaviorTree
{
public:
    BehaviorTree(std::unique_ptr<Node> root);

    int tick();
    int tickCount() const;

    void getActivePath(); // Searching
    void debugView() const; // Traversal

private:
    int m_ticks;
    std::unique_ptr<Node> m_root;
};