#include "ActionNode.hpp"

#include <iostream>

int ActionNode::tick()
{
    ++m_tickCount;

    std::cout << "Action!!" << '\n';

    return 1;
}