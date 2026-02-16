#pragma once

#include "Node.hpp"

class LeafNode: public Node 
{
    public:
        using Node::Node;
        virtual ~LeafNode() = default;

        virtual void print(int depth) const;
        virtual int tick() = 0;

    private:
};