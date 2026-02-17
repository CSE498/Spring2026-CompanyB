#include "DecoratorNode.hpp"

#include <iostream>

void DecoratorNode::addNode(std::unique_ptr<Node> node)
{
    //assert(m_child); // Check for Existing Child
    
    m_child = std::move(node);
}

void DecoratorNode::deleteNode()
{
    m_child = nullptr;
}

void DecoratorNode::print(int depth) const
{
    int indent = 2;

    std::cout << std::string(depth * indent, ' ') << m_name << " (" << m_status << ")" << '\n';
    
    m_child->print(depth + 1);
}

// bool DecoratorNode::getActivePath(std::vector< std::unique_ptr<Node>>& path)
// {
//     if (m_status == -1) 
//     {
//         path.push_back(this);
//         return true;
//     }

//     m_child->getActivePath(path);

//     return false;
// }