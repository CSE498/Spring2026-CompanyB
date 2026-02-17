#pragma once

#include "Node.hpp"

class LeafNode: public Node 
{
    public:
        using Node::Node;
        LeafNode(std::string name, int tickDuration) : Node(name), m_tickDuration(tickDuration) {}

        virtual ~LeafNode() = default;

        virtual void print(int depth) const;
        virtual int tick() = 0;

        virtual std::string getActivePath() = 0;

    protected:
        int m_status{-1};
        int m_tickDuration{};

    private:
};