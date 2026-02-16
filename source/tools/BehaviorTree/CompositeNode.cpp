#include "CompositeNode.hpp"

#include <iostream>
#include <algorithm>

bool CompositeNode::addNode(std::unique_ptr<Node> node)
{
    if (!node) return false; // Check for Nullptr

    m_children.push_back(std::move(node));

    return true;
}

// ATTRIBUTIONS: Used ChatGPT to get the remove_if algorithm implementation

bool CompositeNode::deleteNode(Node* node)
{
    auto delNode = std::remove_if(
                        m_children.begin(),
                        m_children.end(),
                        [node](const std::unique_ptr<Node>& child) {
                            return child.get() == node;
                        }
                    );

    if (delNode == m_children.end()) return false;

    m_children.erase(delNode, m_children.end());

    return true;
}

void CompositeNode::print(int depth) const
{
    std::cout << std::string(depth * 2, ' ') << m_name << '\n';

    for (const auto& child : m_children)
        child->print(depth + 1);
}