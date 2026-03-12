#pragma once

#include "DecoratorNode.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come from my input

/**
 * @brief Decorator node that repeatedly executes its child.
 *
 * A RepeatNode wraps a single child node and re-executes it until failure.
 *
 * The decorator controls how the child's return status
 * influences the overall result.
 */
class RepeatNode: public DecoratorNode
{
    public:
        using DecoratorNode::DecoratorNode;

        Status tick() override {
            ++m_tickCount;

            // Check status of active child
            auto& child = this->getChild();
            if (!child) return Status::Running;

            Status status = child->tick();

            m_status = (status != Status::Failure) ? Status::Running : Status::Failure;

            return m_status;
        };

        int tickCount() const { return m_tickCount; }

        std::string getActivePath() override { 
            auto& child = this->getChild();

            return (child) ? m_name + " - " + child->getActivePath() : m_name;
        };

    private:
        /// Tracks the total number of tick() calls made on this node.
        int m_tickCount{};
};