#include "SequenceNode.hpp"

int SequenceNode::tick()
{
    ++m_tickCount;

    bool isRunning = false;

    int fail = 0;
    int running = -1;
    int pass = 1;

    auto& children = this->getChildren();
    auto& child = *(children.begin() + m_index);

    int status = child->tick();

    switch (status)
    {
        case 1: 
            ++m_index;
            isRunning = true;
            break;
            
        case -1: 
            isRunning = true;
            break;

        case 0: 
            m_status = fail;
            return fail;
            break;
    }

    m_status = (isRunning && m_index < children.size()) ? running : pass;
    
    return m_status;
}

std::string SequenceNode::getActivePath()
{ 
    auto& children = this->getChildren();
    auto& child = *(children.begin() + m_index);

    return m_name + " - " + child->getActivePath();
}