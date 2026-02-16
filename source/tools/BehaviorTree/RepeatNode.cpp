#include "RepeatNode.hpp"

int RepeatNode::tick()
{
    ++m_tickCount;

    auto& child = this->getChild();
    int s = child->tick();

    return (s != 0) ? -1 : 0;
}