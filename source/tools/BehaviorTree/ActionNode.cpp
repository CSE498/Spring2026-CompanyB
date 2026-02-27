#include "ActionNode.hpp"

#include <iostream>

Status ActionNode::tick()
{
    ++m_tickCount;

    std::cout << m_name << '\n';

    m_status = (m_tickDuration > 1) ? Status::Running : Status::Success;

    if ((m_tickDuration > 0)) --m_tickDuration;

    return m_status;
}