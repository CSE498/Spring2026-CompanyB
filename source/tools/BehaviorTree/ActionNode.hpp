#pragma once

#include "LeafNode.hpp"

class ActionNode: public LeafNode
{
    public:
        using LeafNode::LeafNode;

        int tick() override;

        int tickCount() const { return m_tickCount; }

    private:
        int m_tickCount{};
};