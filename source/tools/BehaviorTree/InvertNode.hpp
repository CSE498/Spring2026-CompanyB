#pragma once

#include "DecoratorNode.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come from my input

/**
 * @brief Decorator node that inverts its child's result.
 *
 * An InvertNode (also known as an Inverter) wraps a single child node
 * and reverses its success/failure outcome:
 *
 * - If the child returns Status::Success -> returns Status::Failure.
 * - If the child returns Status::Failure -> returns Status::Success.
 * - If the child returns Status::Running -> returns Status::Running (unchanged).
 *
 * This decorator is commonly used to negate conditions
 * or reverse logical behavior in a behavior tree.
 */
class InvertNode: public DecoratorNode
{
    public:
        using DecoratorNode::DecoratorNode;

        Status tick() override;

        int tickCount() const { return m_tickCount; }

        std::string getActivePath() override;

    private:
        /// Tracks the total number of tick() calls made on this node.
        int m_tickCount{};
};