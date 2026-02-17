#pragma once

#include "DecoratorNode.hpp"

class RepeatNode: public DecoratorNode
{
    public:
        using DecoratorNode::DecoratorNode;

        int tick() override;

        int tickCount() const { return m_tickCount; }

        std::string getActivePath() override;

    private:
        int m_tickCount{};
};