#pragma once

#include "SequenceNode.hpp"
#include "SelectNode.hpp"
#include "InvertNode.hpp"
#include "RepeatNode.hpp"
#include "ActionNode.hpp"

#include <memory>

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come from my input

namespace cse498
{
    /**
     * @brief Represents the root container and execution driver for a Behavior Tree.
     *
     * The BehaviorTree owns the root Node and is responsible for
     * advancing execution through calls to tick(). It also provides
     * utilities for debugging and introspection, such as retrieving
     * the currently active execution path and visualizing the tree.
     */
    class BehaviorTree
    {
    public:
        /**
         * @brief Constructs a BehaviorTree with the given root node.
         *
         * Transfers ownership of the provided root node into the tree.
         *
         * The root must not be nullptr.
         *
         * @param root Unique pointer to the root Node of the tree.
         */
        BehaviorTree(std::unique_ptr<Node> root) : m_root(std::move(root)) { assert(m_root != nullptr); }

        /**
         * @brief Advances the behavior tree by one tick.
         *
         * Calls tick() on the root node, propagating execution
         * through the tree according to node semantics.
         *
         * Increments the internal tick counter.
         *
         * @return The status code returned by the root node
         *         (e.g., Success (1), Failure (0), or Running (-1)).
         */
        Status tick();

        /**
         * @brief Returns the total number of times the tree has been ticked.
         *
         * @return The accumulated tick count.
         */
        int tickCount() const { return m_tickCount; }

        /**
         * @brief Retrieves the currently active execution path.
         *
         * Traverses the tree to determine which nodes are
         * currently active or running and returns a string
         * representation of that path.
         *
         * @return A formatted string representing the active node path.
         */
        std::string getActivePath();

        /**
         * @brief Outputs a debug visualization of the behavior tree.
         *
         * Traverses the tree structure and prints a formatted
         * representation of its nodes and current states.
         *
         * Intended for debugging and development purposes.
         */
        void debugView() const;

    private:
        /// Tracks the total number of tick() calls made on this tree.
        int m_tickCount{};

        /// Owning pointer to the root node of the behavior tree.
        std::unique_ptr<Node> m_root;
    };
}