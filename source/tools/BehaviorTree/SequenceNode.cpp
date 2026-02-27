#include "SequenceNode.hpp"

Status SequenceNode::tick()
{
    ++m_tickCount;

    bool isRunning = false;

    // Check if children are empty
    auto& children = this->getChildren();
    if (children.empty()) return Status::Running;
    
    // Check status of active child
    auto& child = (*(children.begin() + m_index)) ? *(children.begin() + m_index) : *(children.end() - 1);
    Status status = child->tick();

    // Short Circuit if m_status is Status::Success or Status::Failure
    if (m_status == Status::Success || m_status == Status::Failure) return m_status;

    switch (status)
    {
        case Status::Success: 
            ++m_index;
            isRunning = true;
            break;
            
        case Status::Running: 
            isRunning = true;
            break;

        case Status::Failure: 
            m_status = Status::Failure;
            return Status::Failure;
            break;
    }

    if (isRunning && m_index < children.size()) {
        m_status = Status::Running;
    }

    else {
        m_status = Status::Success;
        --m_index;
    }
    
    return m_status;
}

std::string SequenceNode::getActivePath()
{ 
    auto& children = this->getChildren();

    if (children.empty()) return m_name;

    // Check if child is empty
    auto& child = (*(children.begin() + m_index)) ? *(children.begin() + m_index) : *(children.end() - 1);
    return m_name + " - " + child->getActivePath();
}