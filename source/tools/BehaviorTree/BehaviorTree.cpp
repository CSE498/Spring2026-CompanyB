#include "BehaviorTree.hpp"

BehaviorTree::BehaviorTree(std::unique_ptr<Node> root) : m_root(std::move(root))
{

}

void BehaviorTree::getActivePath()
{

}

int BehaviorTree::tick()
{
    return m_root->tick();
}

void BehaviorTree::debugView() const
{
    m_root->print(0);
}