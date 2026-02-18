#pragma once

#include "CompositeNode.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come from my input

/**
 * @brief Composite node that selects the first successful child.
 *
 * A SelectNode (also known as a Selector or Fallback node)
 * ticks its children in order from left to right until:
 *
 * - A child returns Success -> the selector immediately returns Success.
 * - A child returns Running -> the selector returns Running and resumes
 *   from that child on the next tick.
 * - All children return Failure -> the selector returns Failure.
 *
 * The node maintains internal state to remember which child
 * is currently active between ticks.
 */
class SelectNode: public CompositeNode
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