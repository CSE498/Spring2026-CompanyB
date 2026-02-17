#include "ActionNode.hpp"

#include <iostream>

int ActionNode::tick()
{
    ++m_tickCount;

    int pass = 1;
    int running = -1;

    std::cout << m_name << '\n';

    m_status = (m_tickDuration > 1) ? running : pass;

    if ((m_tickDuration > 0)) --m_tickDuration;

    return m_status;
}