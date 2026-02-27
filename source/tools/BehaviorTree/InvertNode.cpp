#include "InvertNode.hpp"

Status InvertNode::tick()
{
    ++m_tickCount;
    
    // Check status of active child
    auto& child = this->getChild();
    Status status = child->tick();

    if (status == Status::Running) 
    {
        m_status = Status::Running;
        return m_status;
    }

    m_status = (status == Status::Success) ? Status::Failure : Status::Success;

    return m_status;
}

std::string InvertNode::getActivePath()
{ 
    auto& child = this->getChild();

    return m_name + " - " + child->getActivePath();
}