#pragma once

#include "LeafNode.hpp"

class ActionNode: public LeafNode
{
    public:
        using LeafNode::LeafNode;

        int tick() override;

        int tickCount() const { return m_tickCount; }

        std::string getActivePath() override { return m_name; }

    private:
        int m_tickCount{};
};