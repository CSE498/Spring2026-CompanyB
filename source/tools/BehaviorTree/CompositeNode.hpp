#pragma once

#include "Node.hpp"

#include <vector>

class CompositeNode: public Node 
{
    public:
        using Node::Node;
        virtual ~CompositeNode() = default;

        bool addNode(std::unique_ptr<Node> node); // Insertion
        bool deleteNode(Node* node); // Deletion

        virtual void print(int depth) const;
        virtual int tick() = 0;

    protected:
        const std::vector< std::unique_ptr<Node>>& getChildren() const 
        {
            return m_children;
        }

    private:
        std::vector< std::unique_ptr<Node>> m_children;
};