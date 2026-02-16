#include "SelectNode.hpp"

int SelectNode::tick()
{
    ++m_tickCount;

    bool isRunning = false;

    for (auto& child : this->getChildren()) {
        int s = child->tick();

        if (s == 1) return 1;
        if (s == -1) isRunning = true;
    }
    
    return (isRunning) ? -1 : 0;
}