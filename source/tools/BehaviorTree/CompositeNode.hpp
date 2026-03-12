#pragma once

#include "Node.hpp"

#include <algorithm>

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come from my input

/**
 * @brief Abstract base class for composite nodes in a Behavior Tree.
 *
 * A CompositeNode manages multiple child nodes and defines how
 * they are executed in relation to each other. Common examples
 * include SequenceNode and SelectNode (Selector), which
 * implement different execution policies over their children.
 *
 * Composite nodes maintain a list of children and propagate ticks
 * according to their specific logic. Derived classes must implement
 * tick() and getActivePath() to define their execution behavior.
 */
class CompositeNode: public Node 
{
    public:
        using Node::Node;
        virtual ~CompositeNode() = default;

        /**
        * @brief Adds a child node to this composite.
        *
        * Ownership of the node is transferred via a unique_ptr.
        *
        * @param node The node to add as a child.
        */
        void addNode(std::unique_ptr<Node> node)
        {
            //assert(node); // Check for Nullptr

            m_children.push_back(std::move(node));
        };

        /**
        * @brief Removes a specific child node from this composite.
        *
        * The child is destroyed when removed.
        *
        * @param node Pointer to the child node to remove.
        */

        // ATTRIBUTIONS: Used ChatGPT to get the remove_if algorithm implementation

        void deleteNode(Node* node) {
            auto delNode = std::remove_if(
                                m_children.begin(),
                                m_children.end(),
                                [node](const std::unique_ptr<Node>& child) {
                                    return child.get() == node;
                                }
                            );

            //assert(delNode == m_children.end());

            m_children.erase(delNode, m_children.end());
        };

        virtual void print(int depth) const {
            int indent = 2;

            std::cout << std::string(depth * indent, ' ') << m_name << " (" << m_status << ")" << '\n';

            for (const auto& child : m_children)
                child->print(depth + 1);
        };

        virtual Status tick() = 0;
        virtual std::string getActivePath() = 0;

    protected:
        /**
        * @brief Provides read-only access to the child nodes.
        *
        * @return A const reference to the vector of child nodes.
        */
        const std::vector< std::unique_ptr<Node>>& getChildren() const 
        {
            return m_children;
        }

    private:
        /// Owning pointers to all child nodes of this composite.
        std::vector< std::unique_ptr<Node>> m_children;
};