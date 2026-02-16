#pragma once

#include "CompositeNode.hpp"

class SequenceNode: public CompositeNode
{
    public:
        using CompositeNode::CompositeNode;

        int tick() override;

        int tickCount() const { return m_tickCount; }

    private:
        int m_tickCount{};
};