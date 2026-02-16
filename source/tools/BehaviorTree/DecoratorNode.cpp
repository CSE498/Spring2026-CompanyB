#include "DecoratorNode.hpp"

#include <iostream>

bool DecoratorNode::addNode(std::unique_ptr<Node> node)
{
    if (m_child) return false; // Check for Existing Child
    
    m_child = std::move(node);

    return true;
}

bool DecoratorNode::deleteNode()
{
    m_child = nullptr;

    return true;
}

void DecoratorNode::print(int depth) const
{
    std::cout << std::string(depth * 2, ' ') << m_name << '\n';
    
    m_child->print(depth + 1);
}