#pragma once

#include "Node.hpp"

class DecoratorNode: public Node 
{
    public:
        using Node::Node;
        virtual ~DecoratorNode() = default;

        void addNode(std::unique_ptr<Node> node); // Insertion
        void deleteNode(); // Deletion

        virtual void print(int depth) const;
        virtual int tick() = 0;

        virtual std::string getActivePath() = 0;

    protected:
        int m_status{-1};

        const std::unique_ptr<Node>& getChild() const 
        {
            return m_child;
        }

    private:
        std::unique_ptr<Node> m_child;
};