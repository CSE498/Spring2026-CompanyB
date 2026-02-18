#include "CompositeNode.hpp"

#include <iostream>
#include <algorithm>

void CompositeNode::addNode(std::unique_ptr<Node> node)
{
    //assert(node); // Check for Nullptr

    m_children.push_back(std::move(node));
}

// ATTRIBUTIONS: Used ChatGPT to get the remove_if algorithm implementation

void CompositeNode::deleteNode(Node* node)
{
    auto delNode = std::remove_if(
                        m_children.begin(),
                        m_children.end(),
                        [node](const std::unique_ptr<Node>& child) {
                            return child.get() == node;
                        }
                    );

    //assert(delNode == m_children.end());

    m_children.erase(delNode, m_children.end());
}

void CompositeNode::print(int depth) const
{
    int indent = 2;

    std::cout << std::string(depth * indent, ' ') << m_name << " (" << m_status << ")" << '\n';

    for (const auto& child : m_children)
        child->print(depth + 1);
}