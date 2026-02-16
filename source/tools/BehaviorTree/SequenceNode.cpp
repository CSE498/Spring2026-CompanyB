#include "SequenceNode.hpp"

int SequenceNode::tick()
{
    ++m_tickCount;

    bool isRunning = false;

    for (auto& child : this->getChildren()) {
        int s = child->tick();
        
        if (s == 0) return s;
        if (s == -1) isRunning = true;
    }
    
    return (isRunning) ? -1 : 1;
}