#pragma once

#include "CompositeNode.hpp"

class SelectNode: public CompositeNode
{
    public:
        using CompositeNode::CompositeNode;

        int tick() override;

        int tickCount() const { return m_tickCount; }

    private:
        int m_tickCount{};
};