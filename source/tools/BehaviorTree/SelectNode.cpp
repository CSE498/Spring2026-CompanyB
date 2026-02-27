#include "SelectNode.hpp"

Status SelectNode::tick()
{
    ++m_tickCount;

    bool isRunning = false;

    // Check status of active child
    auto& children = this->getChildren();
    auto& child = (*(children.begin() + m_index)) ? *(children.begin() + m_index) : *(children.end() - 1);

    Status status = child->tick();

    // Short Circuit if m_status is Status::Success or Status::Failure
    if (m_status == Status::Success || m_status == Status::Failure) return m_status;

    switch (status)
    {
        case Status::Success: 
            m_status = Status::Success;
            return Status::Success;
            break;

        case Status::Running: 
            isRunning = true;
            break;

        case Status::Failure: 
            ++m_index;
            isRunning = true;
            break;
    }

    m_status = (isRunning && m_index < children.size()) ? Status::Running : Status::Failure;
    
    return m_status;
}

std::string SelectNode::getActivePath()
{ 
    auto& children = this->getChildren();
    auto& child = (*(children.begin() + m_index)) ? *(children.begin() + m_index) : *(children.end() - 1);

    return m_name + " - " + child->getActivePath();
}