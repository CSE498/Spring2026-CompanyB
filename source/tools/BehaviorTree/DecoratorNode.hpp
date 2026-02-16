#pragma once

#include "Node.hpp"

class DecoratorNode: public Node 
{
    public:
        using Node::Node;
        virtual ~DecoratorNode() = default;

        bool addNode(std::unique_ptr<Node> node); // Insertion
        bool deleteNode(); // Deletion

        virtual void print(int depth) const;
        virtual int tick() = 0;

    protected:
        const std::unique_ptr<Node>& getChild() const 
        {
            return m_child;
        }

    private:
        std::unique_ptr<Node> m_child;
};