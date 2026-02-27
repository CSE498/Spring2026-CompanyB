#include "InvertNode.hpp"

Status InvertNode::tick()
{
    ++m_tickCount;

    auto& child = this->getChild();
    if (!child) return Status::Running;

    // Check status of active child
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

    return (child) ? m_name + " - " + child->getActivePath() : m_name;
}