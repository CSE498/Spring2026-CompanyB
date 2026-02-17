#pragma once

#include "CompositeNode.hpp"

class SelectNode: public CompositeNode
{
    public:
        using CompositeNode::CompositeNode;

        int tick() override;

        int tickCount() const { return m_tickCount; }

        std::string getActivePath() override;

    private:
        int m_tickCount{};
        unsigned int m_index{};
};