#include "RepeatNode.hpp"

int RepeatNode::tick()
{
    ++m_tickCount;

    int fail = 0;
    int running = -1;

    // Check status of active child
    auto& child = this->getChild();
    int status = child->tick();

    m_status = (status != fail) ? running : fail;

    return m_status;
}

std::string RepeatNode::getActivePath()
{ 
    auto& child = this->getChild();

    return m_name + " - " + child->getActivePath();
}