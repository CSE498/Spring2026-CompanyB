#include "SelectNode.hpp"

int SelectNode::tick()
{
    ++m_tickCount;

    bool isRunning = false;

    int fail = 0;
    int running = -1;
    int pass = 1;

    // Check status of active child
    auto& children = this->getChildren();
    auto& child = *(children.begin() + m_index);

    int status = child->tick();

    switch (status)
    {
        case 1: 
            m_status = pass;
            return pass;
            break;

        case -1: 
            isRunning = true;
            break;

        case 0: 
            ++m_index;
            isRunning = true;
            break;
    }

    m_status = (isRunning && m_index < children.size()) ? running : fail;
    
    return m_status;
}

std::string SelectNode::getActivePath()
{ 
    auto& children = this->getChildren();
    auto& child = *(children.begin() + m_index);

    return m_name + " - " + child->getActivePath();
}