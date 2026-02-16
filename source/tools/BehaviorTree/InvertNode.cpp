#include "InvertNode.hpp"

int InvertNode::tick()
{
    ++m_tickCount;

    auto& child = this->getChild();
    int s = child->tick();

    if (s == -1) return -1;

    return (s == 1) ? 0 : 1;
}