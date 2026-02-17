#include "LeafNode.hpp"

#include <iostream>

void LeafNode::print(int depth) const
{
    std::cout << std::string(depth * 2, ' ') << m_name << " (" << m_status << "): " << m_tickDuration << '\n';
}

// bool LeafNode::getActivePath(std::vector< std::unique_ptr<Node>>& path)
// {
//     if (m_status == -1) 
//     {
//         path.push_back(this);
//         return true;
//     }

//     return false;
// }