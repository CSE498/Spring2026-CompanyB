#pragma once

#include "Node.hpp"

class CompositeNode: public Node 
{
    public:
        using Node::Node;
        virtual ~CompositeNode() = default;

        void addNode(std::unique_ptr<Node> node);
        void deleteNode(Node* node);

        virtual void print(int depth) const;
        virtual int tick() = 0;

        virtual std::string getActivePath() = 0;

    protected:
        int m_status{-1};

        const std::vector< std::unique_ptr<Node>>& getChildren() const 
        {
            return m_children;
        }

    private:
        std::vector< std::unique_ptr<Node>> m_children;
};