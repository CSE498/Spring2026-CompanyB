#pragma once

#include "CompositeNode.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come from my input

/**
 * @brief Composite node that executes its children sequentially.
 *
 * A SequenceNode ticks its children in order from left to right.
 * Execution proceeds until:
 * 
 * - A child returns Failure -> the sequence immediately returns Failure.
 * - A child returns Running -> the sequence returns Running and resumes
 *   from that child on the next tick.
 * - All children return Success -> the sequence returns Success.
 *
 * The node maintains internal state to track which child is currently
 * active between ticks.
 */
class SequenceNode: public CompositeNode
{
    public:
        using CompositeNode::CompositeNode;

        int tick() override;

        int tickCount() const { return m_tickCount; }

        std::string getActivePath() override;

    private:
        /// Tracks the total number of tick() calls made on this node.
        int m_tickCount{};

        /// Index of the currently active child node.
        unsigned int m_index{};
};