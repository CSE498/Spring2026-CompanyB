#include "InvertNode.hpp"

int InvertNode::tick()
{
    ++m_tickCount;

    int fail = 0;
    int running = -1;
    int pass = 1;

    auto& child = this->getChild();
    
    int status = child->tick();

    if (status == running) 
    {
        m_status = running;
        return m_status;
    }

    m_status = (status == pass) ? fail : pass;

    return m_status;
}

std::string InvertNode::getActivePath()
{ 
    auto& child = this->getChild();

    return m_name + " - " + child->getActivePath();
}