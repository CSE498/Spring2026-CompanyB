#pragma once

#include "DecoratorNode.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come from my input

/**
 * @brief Decorator node that inverts its child's result.
 *
 * An InvertNode (also known as an Inverter) wraps a single child node
 * and reverses its success/failure outcome:
 *
 * - If the child returns Success -> returns Failure.
 * - If the child returns Failure -> returns Success.
 * - If the child returns Running -> returns Running (unchanged).
 *
 * This decorator is commonly used to negate conditions
 * or reverse logical behavior in a behavior tree.
 */
class InvertNode: public DecoratorNode
{
    public:
        using DecoratorNode::DecoratorNode;

        int tick() override;

        int tickCount() const { return m_tickCount; }

        std::string getActivePath() override;

    private:
        /// Tracks the total number of tick() calls made on this node.
        int m_tickCount{};
};