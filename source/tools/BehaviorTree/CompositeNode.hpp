#pragma once

#include "Node.hpp"

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
        void addNode(std::unique_ptr<Node> node);

        /**
        * @brief Removes a specific child node from this composite.
        *
        * The child is destroyed when removed.
        *
        * @param node Pointer to the child node to remove.
        */
        void deleteNode(Node* node);

        virtual void print(int depth) const;
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