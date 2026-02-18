#include "LeafNode.hpp"

#include <iostream>

void LeafNode::print(int depth) const
{
    std::cout << std::string(depth * 2, ' ') << m_name << " (" << m_status << "): " << m_tickDuration << '\n';
}