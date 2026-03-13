#pragma once

#include "CompositeNode.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come from my input

/**
 * @brief Composite node that executes its children sequentially.
 *
 * A SequenceNode ticks its children in order from left to right.
 * Execution proceeds until:
 * 
 * - A child returns Status::Failure -> the sequence immediately returns Status::Failure.
 * - A child returns Status::Running -> the sequence returns Status::Running and resumes
 *   from that child on the next tick.
 * - All children return Status::Success -> the sequence returns Status::Success.
 *
 * The node maintains internal state to track which child is currently
 * active between ticks.
 */
class SequenceNode: public CompositeNode
{
    public:
        using CompositeNode::CompositeNode;

        Status tick() override {
            ++m_tickCount;

            bool isRunning = false;

            // Check if children are empty
            auto& children = this->getChildren();
            if (children.empty()) return Status::Running;
            
            // Check status of active child (Pick last child if m_index is out of bounds)
            auto& child = (*(children.begin() + m_index)) ? *(children.begin() + m_index) : *(children.end() - 1);
            Status status = child->tick();

            // Short Circuit if m_status is Status::Success or Status::Failure
            if (m_status == Status::Success || m_status == Status::Failure) return m_status;

            switch (status)
            {
                case Status::Success: 
                    ++m_index;
                    isRunning = true;
                    break;
                    
                case Status::Running: 
                    isRunning = true;
                    break;

                case Status::Failure: 
                    m_status = Status::Failure;
                    return Status::Failure;
                    break;
            }

            if (isRunning && m_index < children.size()) {
                m_status = Status::Running;
            }

            else {
                m_status = Status::Success;
                --m_index;
            }
            
            return m_status;
        };

        int tickCount() const { return m_tickCount; }

        std::string getActivePath() override { 
            auto& children = this->getChildren();

            if (children.empty()) return m_name;

            // Check if child is empty
            auto& child = (*(children.begin() + m_index)) ? *(children.begin() + m_index) : *(children.end() - 1);
            return m_name + " - " + child->getActivePath();
        };

    private:
        /// Tracks the total number of tick() calls made on this node.
        int m_tickCount{};

        /// Index of the currently active child node.
        unsigned int m_index{};
};