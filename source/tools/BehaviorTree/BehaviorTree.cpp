#include "BehaviorTree.hpp"

#include <iostream>

std::string cse498::BehaviorTree::getActivePath()
{
    return m_root->getActivePath();
}

int cse498::BehaviorTree::tick()
{
    ++m_tickCount;
    return m_root->tick();
}

void cse498::BehaviorTree::debugView() const
{
    int depth = 0;
    m_root->print(depth);
}