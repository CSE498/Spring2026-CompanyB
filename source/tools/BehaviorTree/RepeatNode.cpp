#include "RepeatNode.hpp"

Status RepeatNode::tick()
{
    ++m_tickCount;

    // Check status of active child
    auto& child = this->getChild();
    if (!child) return Status::Running;

    Status status = child->tick();

    m_status = (status != Status::Failure) ? Status::Running : Status::Failure;

    return m_status;
}

std::string RepeatNode::getActivePath()
{ 
    auto& child = this->getChild();

    return (child) ? m_name + " - " + child->getActivePath() : m_name;
}